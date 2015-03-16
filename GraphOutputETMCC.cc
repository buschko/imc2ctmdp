#include "GraphOutputETMCC.h"
#include <string>
#include <fstream>
#include <cassert>
#include "State.h"
#include "Transition.h"

void GraphOutputETMCC::writeToFile(Graph* graph, const std::string &,
        const std::string &filename)
{

  // get the Graph ready for export
  graph->prepareForExport();

  graph->debug("Dumping to ETMCC format.");
  graph->debug("OUTPUT %s", filename.c_str());

  std::vector<State*> &states = *graph->getStates();

  // open file
  std::ofstream traFile;
  traFile.open(filename.c_str(), std::ios::out | std::ios::trunc);

  if (!traFile) {
    graph->warn("Error opening file!");
    return;
  }

  // now we have to count the transitions (not very nice)
  unsigned int noTransitions = 0;
  for (std::vector<State*>::const_iterator state = states.begin();
      state != states.end(); ++state)
    noTransitions += (*state)->getTransitions()->size();

  // dump number of States/Transitions
  traFile << "STATES " << states.size() << std::endl
    << "TRANSITIONS " << noTransitions << std::endl << std::endl;

  // write Transitions
  for (std::vector<State*>::const_iterator state = states.begin();
      state != states.end(); ++state) {

    const bool interactiveState = ((*state)->getType() == State::INTERACTIVE);

    for (std::vector<Transition*>::const_iterator trans =
        (*state)->getTransitions()->begin();
        trans != (*state)->getTransitions()->end(); ++trans) {

      assert((*trans)->isInteractive() == interactiveState);
      
      traFile
        // Prefix: 'd' for interactive Transitions, 'r' for markov Transitions
        << (interactiveState ? 'd' : 'r') << " "
        // source State number plus offset
        << (*state)->getNumber() + stateNumberOffset << " "
        // target State number plus offset
        << (*trans)->getTargetState()->getNumber() + stateNumberOffset << " "
        // the Transition rate ("0.0" of interactive Transitions)
        << (interactiveState ? 0.0 : (*trans)->getLabel()->getRate()) << " "
        // Suffix: 'I' for interactive Transitions, 'M' for markov Transitions
        << (interactiveState ? 'I' : 'M')
        // ready, print newLine
        << std::endl;
    }
  }
  
  if (!traFile)
    graph->warn("Warning: It seems as if there was an error while writing "
        "tra file.");
  traFile.close();

  // compute lab filename
  std::string labFilename;
  if ((filename.size() >= 6) &&
      (filename.substr(filename.size() - 4) == ".tra"))
    labFilename = filename.substr(0, filename.size() - 4)+".lab";
  else
    labFilename = filename + ".lab";

  // write the .lab file
  graph->write("lab", labFilename);

}



GraphOutputETMCC::~GraphOutputETMCC()
{
  // nothing to do
}

GraphOutputETMCC::registerClass GraphOutputETMCC::registerObject;

