#include "GraphOutputCtmdp.h"
#include <string>
#include <fstream>
#include <set>
#include <cassert>
#include <iomanip>
#include "State.h"
#include "Transition.h"
#include "Label.h"

void GraphOutputCtmdp::writeToFile(Graph* graph, const std::string &format,
        const std::string &filename)
{

  // get the Graph ready for export
  graph->prepareForExport();

  // if format is "ctmdpi", then the actions should be grouped in the output
  // file
  bool groupActions = (format == "ctmdpi");

  graph->debug("Dumping to %s format.", groupActions ? "CTMDPI" : "CTMDP");
  graph->debug("OUTPUT %s", filename.c_str());

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
  std::ofstream ctmdpFile;
  ctmdpFile.open(filename.c_str(), std::ios::out | std::ios::trunc);

  if (!ctmdpFile) {
    graph->warn("Error opening file!");
    return;
  }

  // dump number of states/transitions
  ctmdpFile << "STATES " << nrInteractiveStates << std::endl
    << "#DECLARATION" << std::endl;

  // Set of all actions
  std::set<std::string> allActions;

  // write out all action labels and all "reach" declarations
  unsigned int toDo = nrInteractiveStates;
  for (std::vector<State*>::const_iterator state = states.begin();
      toDo != 0; ++state, --toDo) {

    // search for new action labels
    for (std::vector<Transition*>::const_iterator trans =
        (*state)->getTransitions()->begin();
        trans != (*state)->getTransitions()->end();
        ++trans) {
      const std::string action = (*trans)->getLabel()->str();
      if (allActions.insert(action).second)
        ctmdpFile << action << std::endl;
    }
  }
  ctmdpFile << "#END" << std::endl;
  // not needed any more...
  allActions.clear();

  // how many transitions had to be left out for avoiding internal
  // nondeterminism?
  unsigned int leftOut = 0;

  // and now the main work...
  toDo = nrInteractiveStates;
  for (std::vector<State*>::const_iterator state = states.begin();
      toDo != 0; ++state, --toDo) {

    // in the .ctmdp-file, there is no internal nondeterminism permitted
    // in this set, we save all seen actions emanating this state to
    // avoid having internal nondeterminism
    std::set<std::string> seenActions;

    for (std::vector<Transition*>::const_iterator trans =
        (*state)->getTransitions()->begin();
        trans != (*state)->getTransitions()->end();
        ++trans) {

      // the probability function to get to another state
      std::map<unsigned int, double> rates;

      State* target = (*trans)->getTargetState();

      const std::string& action = (*trans)->getLabel()->str();

      // some assertions: the Transitions emanating interactive States must
      // be interactive, and their targets must be markov States
      assert((*trans)->isInteractive());
      assert(target->getType() == State::MARKOV);

      for (std::vector<Transition*>::const_iterator mTrans =
          target->getTransitions()->begin();
          mTrans != target->getTransitions()->end();
          ++mTrans) {

        // again some assertions: the Transitions emanating markov States must
        // be markov, and their targets must be interactive States
        assert(!(*mTrans)->isInteractive());
        assert(((*mTrans)->getTargetState()->getType() == State::INTERACTIVE)
            || ((*mTrans)->getTargetState()->getType() == State::NOTDEC));

        assert((*mTrans)->getTargetState()->getNumber() < states.size());

        // add the found rate to the map of actions
        rates[(*mTrans)->getTargetState()->getNumber()] += (*mTrans)->getRate();
      }

      // iterate over the States that are reached by the current action
      // (reached States are unordered (out of map))
      if (groupActions) {
        ctmdpFile << (*state)->getNumber() + stateNumberOffset << " "
          << action << std::endl;
        for (std::map<unsigned int, double>::const_iterator rate =
            rates.begin(); rate != rates.end(); ++rate)
          ctmdpFile << "* " << rate->first + stateNumberOffset << " "
            << std::setprecision(10) << rate->second << std::endl;
      } else {
        if (!seenActions.insert(action).second) {
          #ifdef DEBUG
          printf("Leaving out action \"%s\" on state %d.\n",
              action.c_str(), (*state)->getNumber());
          #endif
          leftOut++;
          continue;
        }
        for (std::map<unsigned int, double>::const_iterator rate =
            rates.begin(); rate != rates.end(); ++rate) {
          
          assert((*state)->getNumber() < states.size());

          ctmdpFile << (*state)->getNumber() + stateNumberOffset << " "
            << rate->first + stateNumberOffset << " " << action << " "
            << std::setprecision(10) << rate->second << std::endl;
        }
      }
    }
  }

  if (leftOut > 0)
    graph->warn("Left out %d interactive transitions for avoiding internal "
        "nondeterminism.", leftOut);

  if (!ctmdpFile)
    graph->warn("Warning: It seems as if there was an error while writing "
        "ctmdp file.");
  ctmdpFile.close();

  // compute lab filename
  std::string labFilename;
  if ((filename.size() >= 6) &&
      (filename.substr(filename.size() - 6) == ".ctmdp"))
    labFilename = filename.substr(0, filename.size() - 6) + ".lab";
  else if ((filename.size() >= 7) &&
      (filename.substr(filename.size() - 7) == ".ctmdpi"))
    labFilename = filename.substr(0, filename.size() - 7) + ".lab";
  else
    labFilename = filename + ".lab";

  // write the .lab file
  graph->write("lab", labFilename);

}

GraphOutputCtmdp::~GraphOutputCtmdp()
{
  // nothing to do
}

GraphOutputCtmdp::registerClass GraphOutputCtmdp::registerObject;

