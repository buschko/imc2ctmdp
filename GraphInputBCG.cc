#include "GraphInputBCG.h"
#include <string>
#include <cassert>
#include <bcg_user.h>
#include "Graph.h"
#include "State.h"
#include "Transition.h"

Graph* GraphInputBCG::readFromFile(const std::string &,
    const std::string &filename)
{
  // the BCG object
  BCG_TYPE_OBJECT_TRANSITION bcg_graph;

  BCG_INIT();

  char* filename_char = new char[filename.length()+1];
  filename.copy(filename_char, filename.length());
  filename_char[filename.length()] = '\0';
  
  BCG_OT_READ_BCG_BEGIN(filename_char, &bcg_graph, 0);

  delete [] filename_char;

  unsigned int noStates      = BCG_OT_NB_STATES(bcg_graph);
  unsigned int noTransitions = BCG_OT_NB_EDGES(bcg_graph);
  unsigned int noLabels      = BCG_OT_NB_LABELS(bcg_graph);
  
  // create new Graph object
  Graph* graph = new Graph();

  // output some debug information
  graph->debug("########################################");
  graph->debug("INPUT statistics:");
  graph->debug("  %-23s%15d", "Number of states:", noStates);
  graph->debug("  %-23s%15d", "Number of transitions:", noTransitions);
  graph->debug("########################################");

  // read and save comment
  BCG_TYPE_C_STRING bcg_comment;
  BCG_READ_COMMENT (BCG_OT_GET_FILE (bcg_graph), &bcg_comment);
  std::string comment(bcg_comment);
  comment.append(" (converted by imc2ctmdp)");
  graph->setComment(bcg_comment);
  comment.clear();

  // reserve memory for all states
  std::vector<State*> &states = *graph->getStates();
  states.resize(noStates);
  unsigned int nr = 0;
  for (std::vector<State*>::iterator it = states.begin();
      it != states.end(); ++it) {
    *it = new State();
    (*it)->setNumber(++nr);
  }

  // set pointer to initial state
  assert(BCG_OT_INITIAL_STATE (bcg_graph) < noStates);
  graph->setInitialState(states[BCG_OT_INITIAL_STATE(bcg_graph)]);

  // store label pointers by label numbers (for better performance, because
  // you don't have to ask the BCG every time)
  std::vector<Label*> labels(noLabels, NULL);

  // get the label pointer for 'theAction'
  Label* actionLabelPtr = graph->getLabelPtr(graph->getAction(), true);

  // read all transitions from particular state and decide if Markov or
  // Interactive
  // all transitions and states are stored in memory
  
  BCG_TYPE_STATE_NUMBER bcg_state_1, bcg_state_2;
  BCG_TYPE_LABEL_NUMBER bcg_label_number;

  unsigned int removed(0);

  BCG_OT_ITERATE_PLN (bcg_graph, bcg_state_1, bcg_label_number, bcg_state_2) {

    assert(bcg_state_1 < noStates);
    assert(bcg_state_2 < noStates);
    assert(bcg_label_number < noLabels);

    State *&targetState = states[bcg_state_2],
          *&sourceState = states[bcg_state_1];

    // get transition label
    Label** labelPtr = &labels[bcg_label_number];
    if (!*labelPtr)
      *labelPtr = graph->getLabelPtr(
          BCG_OT_LABEL_STRING(bcg_graph, bcg_label_number), true);

    // if label is 'theAction', then just mark the State and ignore the Transition
    if (*labelPtr == actionLabelPtr) {
      sourceState->setMark(true);
      continue;
    }

    // memory for the new transition
    // set label and target state
    Transition* newTransition = new Transition(targetState, *labelPtr);

    // prove that the new Transition doesn't cause an interactive cycle
    if (newTransition->isInteractive()
        && graph->getCycleSearch()
        && graph->reachable(targetState, sourceState, true)) {
      #ifdef DEBUG
      std::vector<Transition*> cycle = graph->getPath(targetState,
          sourceState, true);
      cycle.push_back(newTransition);
      printf("Deleting Transition \"%s\" from %d to %d because of this "
          "cycle:\n   %d", newTransition->getLabel()->str().c_str(),
          sourceState->getNumber(), targetState->getNumber(),
          targetState->getNumber());
      for (std::vector<Transition*>::iterator it = cycle.begin();
          it != cycle.end(); ++it)
        printf(" --%s--> %d", (*it)->getLabel()->str().c_str(),
            (*it)->getTargetState()->getNumber());
      printf("\n");
      #endif
      delete newTransition;
      ++removed;
      continue;
    }

    // determine state type
    sourceState->determineStateType(newTransition);
    
    // add transition to state
    sourceState->addTransition(newTransition);

  } BCG_OT_END_ITERATE;

  if (removed > 0)
    graph->warn("%d transitions removed because they would have caused interactive cycles", removed);

  // close input file
  BCG_OT_READ_BCG_END (&bcg_graph);

  return graph;

}

GraphInputBCG::~GraphInputBCG()
{
  // nothing to do here
}

GraphInputBCG::registerClass GraphInputBCG::registerObject;

