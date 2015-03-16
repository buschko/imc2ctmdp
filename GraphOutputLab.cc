#include "GraphOutputLab.h"
#include <string>
#include <fstream>
#include <set>
#include <cassert>
#include "State.h"
#include "Transition.h"
#include "Label.h"

const char* GraphOutputLab::reachLabel = "reach";
const char* GraphOutputLab::absorbingLabel = "absorbing";

void GraphOutputLab::writeToFile(Graph* graph, const std::string &,
        const std::string &filename)
{
  graph->debug("Writing lab file.");
  graph->debug("OUTPUT %s", filename.c_str());

  // get the Graph ready for export
  graph->prepareForExport();

  std::vector<State*> &states = *graph->getStates();

  std::vector<bool> markedStates, absorbingStates;
  unsigned int absorbing = 0;

  bool searchAbsorbing = Graph::isSearchForAbsorbingStates();

  for (std::vector<State*>::const_iterator state = states.begin();
      state != states.end(); ++state) {
    const State* const &curState = *state;
    if (curState->getType() == State::MARKOV) {
#ifndef NDEBUG
      for (; state != states.end(); ++state)
        assert((*state)->getType() == State::MARKOV);
#endif
      break;
    }

    if (curState->getMark()) {
      if (markedStates.size() <= curState->getNumber())
        markedStates.resize(curState->getNumber() + 1);
      markedStates[curState->getNumber()] = true;
    }

    if (searchAbsorbing) {
      bool isAbsorbing = true;
      for (std::vector<Transition*>::const_iterator out1 =
          curState->getTransitions()->begin();
          isAbsorbing && (out1 != curState->getTransitions()->end());
          ++out1) {
        State* target1 = (*out1)->getTargetState();
        for (std::vector<Transition*>::const_iterator out2 =
            target1->getTransitions()->begin();
            out2 != target1->getTransitions()->end();
            ++out2)
          if ((*out2)->getTargetState() != curState) {
            isAbsorbing = false;
            break;
          }
      }
      if (isAbsorbing) {
        if (absorbingStates.size() <= curState->getNumber())
          absorbingStates.resize(curState->getNumber() + 1);
        absorbingStates[curState->getNumber()] = true;
        absorbing++;
      }
    }
  }

  if (searchAbsorbing)
    graph->debug("%d absorbing states found.", absorbing);

  
  // open file
  std::ofstream labFile;
  labFile.open(filename.c_str(), std::ios::out | std::ios::trunc);

  if (!labFile) {
    graph->warn("Error opening file!");
    return;
  }

  // write head of lab-File
  labFile << "#DECLARATION" << std::endl
          << reachLabel << std::endl;
  if (absorbing > 0)
    labFile << absorbingLabel << std::endl;
  labFile << "#END" << std::endl;

  unsigned int max = markedStates.size();
  if (absorbingStates.size() > max)
    max = absorbingStates.size();

  for (unsigned int i = 0; i < max; ++i) {
    bool isMarked    = (i <    markedStates.size()) &&    markedStates[i];
    bool isAbsorbing = (i < absorbingStates.size()) && absorbingStates[i];

    if (isMarked || isAbsorbing) {
      labFile << i + stateNumberOffset;
      if (isMarked)
        labFile << " " << reachLabel;
      if (isAbsorbing)
        labFile << " " << absorbingLabel;
      labFile << std::endl;
    }
  }
  if (!labFile)
    graph->warn("Warning: It seems as if there was an error while writing "
        "lab file.");
  labFile.close();

}

GraphOutputLab::~GraphOutputLab()
{
  // nothing to do
}

GraphOutputLab::registerClass GraphOutputLab::registerObject;

