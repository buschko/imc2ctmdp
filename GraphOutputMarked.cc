#include "GraphOutputMarked.h"
#include <string>
#include <fstream>
#include <cassert>
#include "State.h"
#include "Transition.h"

void GraphOutputMarked::writeToFile(Graph* graph, const std::string &,
        const std::string &filename)
{
  // get the Graph ready for export
  graph->prepareForExport();

  std::vector<State*> &states = *graph->getStates();

  // get number of interactive States
  unsigned int nrInteractiveStates(0);
  for (std::vector<State*>::const_iterator state = states.begin();
      state != states.end(); ++state) {
    if ((*state)->getType() == State::MARKOV) {
#ifndef NDEBUG
      for (; state != states.end(); ++state)
        assert((*state)->getType() == State::MARKOV);
#endif
      break;
    }
    ++nrInteractiveStates;
  }

  // open file
  std::ofstream markedFile;
  markedFile.open(filename.c_str(), std::ios::out | std::ios::trunc);

  if (!markedFile) {
    graph->warn("Error opening file!");
    return;
  }

  // iterate over States and write out the marked ones
  for (std::vector<State*>::const_iterator state = states.begin();
      nrInteractiveStates != 0; ++state, --nrInteractiveStates)
    if ((*state)->getMark())
      markedFile << (*state)->getNumber() << std::endl;

  if (!markedFile)
    graph->warn("Warning: It seems as if there was an error while writing "
        ".marked file.");
  markedFile.close();
}

GraphOutputMarked::~GraphOutputMarked()
{
  // nothing to do
}

GraphOutputMarked::registerClass GraphOutputMarked::registerObject;

