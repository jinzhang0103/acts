// This file is part of the Acts project.
//
// Copyright (C) 2019 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ReconTruthBase.hpp"

#include <boost/program_options.hpp>

#include "ActsExamples/Digitization/HitSmearing.hpp"
#include "ActsExamples/EventData/SimHit.hpp"
#include "ActsExamples/EventData/SimParticle.hpp"
#include "ActsExamples/Fitting/FittingAlgorithm.hpp"
#include "ActsExamples/Framework/RandomNumbers.hpp"
#include "ActsExamples/Framework/Sequencer.hpp"
#include "ActsExamples/Framework/WhiteBoard.hpp"
#include "ActsExamples/Geometry/CommonGeometry.hpp"
#include "ActsExamples/Io/Csv/CsvOptionsReader.hpp"
#include "ActsExamples/Io/Performance/TrackFinderPerformanceWriter.hpp"
#include "ActsExamples/Io/Performance/TrackFitterPerformanceWriter.hpp"
#include "ActsExamples/Io/Root/RootTrajectoryWriter.hpp"
#include "ActsExamples/Options/CommonOptions.hpp"
#include "ActsExamples/Plugins/BField/BFieldOptions.hpp"
#include "ActsExamples/TruthTracking/ParticleSmearing.hpp"
#include "ActsExamples/TruthTracking/TruthTrackFinder.hpp"
#include "ActsExamples/Utilities/Options.hpp"
#include "ActsExamples/Utilities/Paths.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"

using namespace Acts::UnitLiterals;

/// @brief Setup for the Truth Reconstruction
///
/// @param variables The boost variable map to resolve
/// @param sequencer The framework sequencer
/// @param tGeometry The TrackingGeometry for the tracking setup
/// truth reco
void
ActsExamples::setupReconTruth(
    const ActsExamples::Options::Variables&                 variables,
    ActsExamples::Sequencer&                                sequencer,
    std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry)
{

  // Read the log level
  Acts::Logging::Level logLevel = ActsExamples::Options::readLogLevel(variables);
  // Output path
  auto outputDir
      = ActsExamples::ensureWritableDirectory(variables["output-dir"].as<std::string>());
  auto rnd = std::make_shared<ActsExamples::RandomNumbers>(
      ActsExamples::Options::readRandomNumbersConfig(variables));

  auto magneticField = ActsExamples::Options::readBField(variables);

  // Create smeared measurements
  ActsExamples::HitSmearing::Config hitSmearingCfg;
  hitSmearingCfg.inputSimulatedHits    = "hits";
  hitSmearingCfg.outputSourceLinks     = "sourcelinks";
  hitSmearingCfg.outputHitParticlesMap = "hit_particles_map";
  hitSmearingCfg.sigmaLoc0             = 25_um;
  hitSmearingCfg.sigmaLoc1             = 100_um;
  hitSmearingCfg.randomNumbers         = rnd;
  hitSmearingCfg.trackingGeometry      = trackingGeometry;
  sequencer.addAlgorithm(
      std::make_shared<ActsExamples::HitSmearing>(hitSmearingCfg, logLevel));

  // The fitter needs the measurements (proto tracks) and initial
  // track states (proto states). The elements in both collections
  // must match and must be created from the same input particles.
  const auto& inputParticles = "particles_initial";
  // Create truth tracks
  ActsExamples::TruthTrackFinder::Config trackFinderCfg;
  trackFinderCfg.inputParticles       = inputParticles;
  trackFinderCfg.inputHitParticlesMap = hitSmearingCfg.outputHitParticlesMap;
  trackFinderCfg.outputProtoTracks    = "prototracks";
  sequencer.addAlgorithm(
      std::make_shared<ActsExamples::TruthTrackFinder>(trackFinderCfg, logLevel));
  // Create smeared particles states
  ActsExamples::ParticleSmearing::Config particleSmearingCfg;
  particleSmearingCfg.inputParticles        = inputParticles;
  particleSmearingCfg.outputTrackParameters = "smearedparameters";
  particleSmearingCfg.randomNumbers         = rnd;
  // Gaussian sigmas to smear particle parameters
  particleSmearingCfg.sigmaD0    = 20_um;
  particleSmearingCfg.sigmaD0PtA = 30_um;
  particleSmearingCfg.sigmaD0PtB = 0.3 / 1_GeV;
  particleSmearingCfg.sigmaZ0    = 20_um;
  particleSmearingCfg.sigmaZ0PtA = 30_um;
  particleSmearingCfg.sigmaZ0PtB = 0.3 / 1_GeV;
  particleSmearingCfg.sigmaPhi   = 1_degree;
  particleSmearingCfg.sigmaTheta = 1_degree;
  particleSmearingCfg.sigmaPRel  = 0.01;
  particleSmearingCfg.sigmaT0    = 1_ns;
  sequencer.addAlgorithm(
      std::make_shared<ActsExamples::ParticleSmearing>(particleSmearingCfg, logLevel));

  // setup the fitter
  ActsExamples::FittingAlgorithm::Config fitter;
  fitter.inputSourceLinks = hitSmearingCfg.outputSourceLinks;
  fitter.inputProtoTracks = trackFinderCfg.outputProtoTracks;
  fitter.inputInitialTrackParameters
      = particleSmearingCfg.outputTrackParameters;
  fitter.outputTrajectories = "trajectories";
  fitter.fit                = ActsExamples::FittingAlgorithm::makeFitterFunction(
      trackingGeometry, magneticField);
  sequencer.addAlgorithm(
      std::make_shared<ActsExamples::FittingAlgorithm>(fitter, logLevel));

  // write tracks from fitting
  ActsExamples::RootTrajectoryWriter::Config trackWriter;
  trackWriter.inputParticles    = inputParticles;
  trackWriter.inputTrajectories = fitter.outputTrajectories;
  trackWriter.outputDir         = outputDir;
  trackWriter.outputFilename    = "tracks.root";
  trackWriter.outputTreename    = "tracks";
  sequencer.addWriter(
      std::make_shared<ActsExamples::RootTrajectoryWriter>(trackWriter, logLevel));

  // write reconstruction performance data
  ActsExamples::TrackFinderPerformanceWriter::Config perfFinder;
  perfFinder.inputParticles       = inputParticles;
  perfFinder.inputHitParticlesMap = hitSmearingCfg.outputHitParticlesMap;
  perfFinder.inputProtoTracks     = trackFinderCfg.outputProtoTracks;
  perfFinder.outputDir            = outputDir;
  sequencer.addWriter(
      std::make_shared<ActsExamples::TrackFinderPerformanceWriter>(perfFinder, logLevel));
  ActsExamples::TrackFitterPerformanceWriter::Config perfFitter;
  perfFitter.inputParticles    = inputParticles;
  perfFitter.inputTrajectories = fitter.outputTrajectories;
  perfFitter.outputDir         = outputDir;
  sequencer.addWriter(
      std::make_shared<ActsExamples::TrackFitterPerformanceWriter>(perfFitter, logLevel));
}
