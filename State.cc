#include "State.h"
#include <cassert>
#include "Transition.h"
#include "Graph.h"

// State construktor
State::State()
  : markovSuccFinished(false), type(NOTDEC), mark(false), interactivePred(NULL)
{
}

// State destructor
State::~State()
{
  // delete all Transitions emanating this state
  for (std::vector<Transition*>::const_iterator it =
      transitions.begin(); it != transitions.end(); ++it)
    delete *it;
  // clear the vectors
  transitions.clear();
}

void State::getMarkovSuccs(const bool &computeLabels)
{
  // this method should only be called for INTERACTIVE states !!
  assert((getType() == INTERACTIVE) || (getType() == HYBRID));

  if (markovSuccFinished)
    return;

  markovSuccFinished = true;

  // now all INTERACTIVE transitions are cut off, and transitions to MARKOV
  // states are searched
  std::vector<Transition*> newTransitions;

  for (std::vector<Transition*>::const_iterator trans = transitions.begin();
      trans != transitions.end(); ++trans) {

    if (!((*trans)->isInteractive())) {
      // a MARKOV transition emanating HYBRID state is cut off
      delete *trans;
      continue;
    }

    State *successor = (*trans)->getTargetState();

    // if MARKOV successor or sink state, keep the transition
    if ((successor->getType() == MARKOV) || 
        (successor->getType() == NOTDEC)) {
      if (!computeLabels)
        (*trans)->setLabel("DFS");
      newTransitions.push_back(*trans);
    } else {
      // append all MARKOV succs of successor to state and prepend the
      // transition label to the "word" to the MARKOV succ

      // there should be no INTERACTIVE self-loop
      assert(successor != this);

      // ignore INTERACTIVE self-loops
      if (successor == this)
        continue;

      // first compute MARKOV successors for the state
      successor->getMarkovSuccs(computeLabels);

      for (std::vector<Transition*>::const_iterator it =
          successor->getTransitions()->begin();
          it != successor->getTransitions()->end(); ++it) {
        Transition* newTrans;
        if (computeLabels) {
          if ((*it)->isTau())
            newTrans = new Transition((*it)->getTargetState(), (*trans)->getLabel());
          else {
            newTrans = new Transition(**it);
            if (!(*trans)->isTau())
              newTrans->setLabel(newTrans->getLabel()->prepend((*trans)->getLabel()));
          }
        } else
          newTrans = new Transition(**it);
        newTransitions.push_back(newTrans);
      }

      // now the transition can be deleted, because it's from INTERACTIVE
      // state to INTERACTIVE state
      delete *trans;
    }
  }
  // now swap vectors `transitions` and `newTransitions`
  newTransitions.swap(transitions);
  newTransitions.clear();

  // if one of the MARKOV successors is marked, also mark this State
  if (!mark)
    for (std::vector<Transition*>::const_iterator it = transitions.begin();
        it != transitions.end(); ++it)
      if ((*it)->getTargetState()->getMark()) {
        mark = true;
        break;
      }
}

void State::determineStateType(const Transition *const &newTransition)
{
  // set state encontered statetype if interactive transition is given
  if (newTransition->isInteractive()) {
    // first set stateType
    switch (this->type) {
      case NOTDEC: type = INTERACTIVE; break;
      case MARKOV: type = HYBRID; break;
      default:     return;
    }
  } else {
    switch (this->type) {
      case NOTDEC:      type = MARKOV; break;
      case INTERACTIVE: type = HYBRID; break;
      default:          return;
    }
  }
}

void State::addTransition(Transition *const &newTransition)
{
  transitions.push_back(newTransition);
}

std::vector<Transition*>::iterator State::removeTransition(
    std::vector<Transition*>::iterator &remove)
{
  return transitions.erase(remove);
}

State::stateType State::getType() const
{
  return type;
}

void State::setType(const stateType &newType)
{
  type = newType;
}

unsigned int State::getNumber() const
{
  return number;
}

void State::setNumber(const unsigned int &newNumber)
{
  number = newNumber;
}

void State::setInteractivePred(State *const &newInteractivePred)
{
  interactivePred = newInteractivePred;
}

State* State::getInteractivePred() const
{
  return interactivePred;
}

std::vector<Transition*>* State::getTransitions()
{
  return &transitions;
}

const std::vector<Transition*>* State::getTransitions() const
{
  return &transitions;
}

bool State::getMark() const
{
  return mark;
}

void State::setMark(const bool &newMark)
{
  mark = newMark;
}

bool State::compareByNr(State* firstState, State* secondState)
{
  return firstState->getNumber() < secondState->getNumber();
}

