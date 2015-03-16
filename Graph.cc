#include "Graph.h"
#include <sstream>
#include <set>
#include <map>
#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <stdlib.h>
#include <string.h>
#ifdef DEBUG
  #include <deque>
#endif
#include <iomanip>
#include "State.h"
#include "Transition.h"
#include "imc2ctmdp.h"
#include "Label.h"
#include "GraphInput.h"
#include "GraphOutput.h"

// static class members
bool Graph::cycleSearch = true;
bool Graph::searchForAbsorbingStates = false;
char* Graph::colorDebug   = "\033[32m";
char* Graph::colorWarning = "\033[31m";
char* Graph::colorReset   = "\033[0m";
std::string Graph::theAction;
std::map<std::string, Label*> Graph::labels;
const double Graph::uniformEpsilon = 1e-8;

Graph::Graph()
  : initialState(NULL), readyForExport(false)
{
}

Graph::~Graph()
{
  // delete all states
  for (std::vector<State*>::const_iterator it = states.begin();
      it != states.end(); ++it)
    delete *it;
  states.clear();
  initialState = NULL;
  // delete labels
  for (std::map<std::string, Label*>::const_iterator it =
      labels.begin(); it != labels.end(); ++it)
    delete it->second;
  labels.clear();
}

Graph* Graph::read(const std::string &format, const std::string &filename)
  throw(std::runtime_error)
{
  // get reader for the given format
  GraphInput* reader = formatRegistry<GraphInput>::getInstance()->get(format);
  // throw exception if the format is unknown
  if (reader == NULL) {
    std::ostringstream error;
    error << "Input format \"" << format << "\" unknown (for filename \""
      << filename << "\")";
    throw std::runtime_error(error.str().c_str());
  }
  // read the Graph from the given filename
  return reader->readFromFile(format, filename);
}

void Graph::write(const std::string &format, const std::string &filename)
  throw (std::runtime_error)
{
  // get writer for the given format
  GraphOutput* writer = formatRegistry<GraphOutput>::getInstance()->get(format);
  // throw exception if the format is unknown
  if (writer == NULL) {
    std::ostringstream error;
    error << "Output format \"" << format << "\" unknown (for filename \""
      << filename << "\")";
    throw std::runtime_error(error.str().c_str());
  }
  // write the Graph to the given filename
  return writer->writeToFile(this, format, filename);
}

void Graph::transformImcToCtmdp(const bool &computeLabels)
{
  // for statistics
  unsigned int imcInteractive(0), imcMarkov(0), imcHybrid(0), imcDeadlock(0),
               imcITrans(0), imcMTrans(0),
               ctmdpInteractive(0), ctmdpMarkov(0), ctmdpDeadlock(0),
               ctmdpITrans(0), ctmdpMTrans(0),
               ctmdpMaxNondet(0), ctmdpMaxFanout(0);

  // compute statistics
  for (std::vector<State*>::const_iterator state = states.begin();
    state != states.end(); ++state) {
    // add nr of interactive/markov transitions
    for (std::vector<Transition*>::const_iterator trans =
        (*state)->getTransitions()->begin();
        trans != (*state)->getTransitions()->end();
        ++trans)
      if ((*trans)->isInteractive())
        ++imcITrans;
      else
        ++imcMTrans;
    switch ((*state)->getType()) {
      case State::HYBRID:
        (*state)->setType(State::INTERACTIVE);
        ++imcHybrid;
        break;
      case State::INTERACTIVE:
        ++imcInteractive;
        break;
      case State::MARKOV:
        ++imcMarkov;
        break;
      default:
        ++imcDeadlock;
        break;
    }
  }

  // We compute reachable MARKOV states for each INTERACTIVE state.
  // BTW store all states with MARKOV predecessor
  std::set<State*> hasMarkovPred;
  std::vector<State*> newStates;
  for (std::vector<State*>::const_iterator state = states.begin();
    state != states.end(); ++state) {
    switch ((*state)->getType()) {
      case State::INTERACTIVE:
        (*state)->getMarkovSuccs(computeLabels);
        break;
      case State::MARKOV:
        // for each successor
        for (std::vector<Transition*>::const_iterator trans =
            (*state)->getTransitions()->begin();
            trans != (*state)->getTransitions()->end(); ++trans) {

          State* targetState = (*trans)->getTargetState();
          // if MARKOV ==> insert new interactive state
          if (targetState->getType() != State::MARKOV) {
            hasMarkovPred.insert((*trans)->getTargetState());
            continue;
          }

          // do we need a new state?
          if (targetState->getInteractivePred())
            (*trans)->setTargetState(targetState->getInteractivePred());
          else {
            // targetState has no interactive successor so far
            State* newState = new State();
            newStates.push_back(newState);
            newState->setType(State::INTERACTIVE);
            newState->addTransition(new Transition(targetState, "i"));
            if (targetState->getMark())
              newState->setMark(true);
            (*trans)->setTargetState(newState);
            // now the target State has an interactive predecessor
            targetState->setInteractivePred(newState);
          }
        }
        break;
      default:
        break;
    }
  }

  // Now delete all INTERACTIVE states with no MARKOV predecessor
  for (std::vector<State*>::iterator state = states.begin();
      state != states.end(); ++state)
    if (((*state)->getType() == State::INTERACTIVE)
        && (!hasMarkovPred.erase(*state))
        && (*state != initialState))
      delete *state;
    else
      newStates.push_back(*state);
  // swap vector newStates with states
  // then clear newStates (old `states` vector)
  newStates.swap(states);
  newStates.clear();
  hasMarkovPred.clear();


  // check if initial state is MARKOV or INTERACTIVE/HYBRID
  if (initialState->getType() == State::MARKOV) {
    // if it is MARKOV create new initial state and append a transition
    State* newState = new State();
    newState->addTransition(new Transition(initialState, "i"));
    newState->setType(State::INTERACTIVE);
    newState->setMark(initialState->getMark());
    initialState = newState;
    states.push_back(newState);
  }

  // statistics after the transformation
  for (std::vector<State*>::const_iterator state = states.begin();
      state != states.end(); ++state) {
    const unsigned int noTrans = (*state)->getTransitions()->size();

    switch ((*state)->getType()) {
      case State::INTERACTIVE:
        ++ctmdpInteractive;
        ctmdpITrans += noTrans;
        if (noTrans > ctmdpMaxNondet)
          ctmdpMaxNondet = noTrans;
        break;
      case State::MARKOV:
        ++ctmdpMarkov;
        ctmdpMTrans += noTrans;
        if (noTrans > ctmdpMaxFanout)
          ctmdpMaxFanout = noTrans;
        break;
      default:
        ++ctmdpDeadlock;
        // assure that no hybrid states exist any more
        assert((*state)->getType() == State::NOTDEC);
        break;
    }
  }


  // output the statistics
  debug("######################################################");
  const char* f = "  %-12s%20d%20d";
  debug("  %12s%20s%20s", "", "IMC statistics", "CTMDP statistics");
  debug("STATES");
  debug(f, "Interactive", imcInteractive, ctmdpInteractive);
  debug(f, "Markov", imcMarkov, ctmdpMarkov);
  debug(f, "Hybrid", imcHybrid, 0);
  debug(f, "Deadlock", imcDeadlock, ctmdpDeadlock);
  debug("TRANSITIONS");
  debug(f, "Interactive", imcITrans, ctmdpITrans);
  debug(f, "Markov", imcMTrans, ctmdpMTrans);
  debug("CTMDP");
  debug("  %-32s%20d", "Maximal degree of nondeterminism", ctmdpMaxNondet);
  debug("  %-32s%20d", "Maximal fanout", ctmdpMaxFanout);
  debug("######################################################");
}

void Graph::deleteUnreachable()
{
  // search for reachable States

  std::set<State*> reached;
  std::vector<State*> toHandle;
  toHandle.push_back(initialState);
  reached.insert(initialState);
  while (!toHandle.empty()) {
    State* cur = toHandle.back();
    toHandle.pop_back();
    for (std::vector<Transition*>::const_iterator trans = cur->getTransitions()->begin();
        trans != cur->getTransitions()->end(); ++trans)
      // if it's not in `reached`, insert into `toHandle`
      if (reached.insert((*trans)->getTargetState()).second) // if it was inserted...
        toHandle.push_back((*trans)->getTargetState());
  }

  // if all States are reached, then we are ready
  if (reached.size() == states.size()) {
    debug("No unreachable states.");
    return;
  }

  // now delete all States that are not reached from initialState
  std::vector<State*> newStates;
  newStates.reserve(reached.size());
  for (std::vector<State*>::const_iterator state = states.begin();
      state != states.end(); ++state)
    if (reached.find(*state) == reached.end())
      delete *state;
    else
      newStates.push_back(*state);

  warn("Deleted %d unreachable states.", (states.size() - newStates.size()));

  // swap `newStates` into `states`
  newStates.swap(states);
  newStates.clear();
}

void Graph::numberStates() const
{
  // order: 1. initial State, 2. INTERACTIVE States, 3. all the Rest
  initialState->setNumber(0);
  unsigned int nextStateNr(0);

  // number INTERACTIVE states
  for (std::vector<State*>::const_iterator state = states.begin();
      state != states.end(); ++state)
    if (((*state)->getType() != State::MARKOV) && (*state != initialState))
      (*state)->setNumber(++nextStateNr);

  // and now the MARKOV states
  for (std::vector<State*>::const_iterator state = states.begin();
      state != states.end(); ++state)
    if (((*state)->getType() == State::MARKOV) && (*state != initialState))
      (*state)->setNumber(++nextStateNr);

  if (nextStateNr != states.size()-1) {
    warn("Warning: The initial state seems to be invalid");
  }
}

void Graph::sortStatesByNr()
{
  std::vector<State*> newStates(states.size());
  for (std::vector<State*>::const_iterator state = states.begin();
    state != states.end(); ++state) {
    const unsigned int &number = (*state)->getNumber();
    if (number >= states.size()) {
      warn("in function sortStatesByNr(): states are not numbered sequently.");
      return;
    }
    newStates[number] = *state;
  }
  newStates.swap(states);
}

Label* Graph::getLabelPtr(const std::string* label)
{
  std::map<std::string, Label*>::const_iterator found =
    labels.find(*label);

  if (found != labels.end())
    return found->second;
  
  Label* newLP;
  try {
    newLP = Label::create(*label);
  } catch (std::runtime_error e) {
    warn("Error creating label \"%s\": %s", label->c_str(), e.what());
    exit(-1);
  }
  labels.insert(std::make_pair(*label, newLP));
  return newLP;
}

Label* Graph::getLabelPtr(const std::string &label, bool quote)
{
  if (quote) {
    std::string quoted = Label::quote(label);
    return getLabelPtr(&quoted);
  } else {
    return getLabelPtr(&label);
  }
}


// needed for Graph::checkInteractiveCycle()
bool _GraphCheckInteractiveCycleDFS(const State *state,
    std::map<const State*, std::pair<bool, bool> > &type)
{
  std::map<const State*, std::pair<bool, bool> >::iterator t = type.find(state);
  if (t == type.end())
    t = type.insert(std::make_pair(state, std::make_pair(false, false))).first;

  if (t->second.first)
    return false; // state is ready
  if (t->second.second)
    return true;  // found cycle

  t->second.second = true;  // state is now visited
  for (std::vector<Transition*>::const_iterator trans =
      state->getTransitions()->begin();
      trans != state->getTransitions()->end(); ++trans)
    if ((*trans)->isInteractive()
        && ((*trans)->getTargetState()->getType() != State::MARKOV)
        && (_GraphCheckInteractiveCycleDFS((*trans)->getTargetState(), type)))
      return true;
  t->second.first = true; // state is ready

  return false;
}

bool Graph::checkInteractiveCycle(const bool &warnCycles) const
{
  for (std::vector<State*>::const_iterator it = states.begin();
      it != states.end(); ++it) {
    if (((*it)->getType() == State::INTERACTIVE)
      || ((*it)->getType() == State::HYBRID)) {
      // first entry: finished, second: visited
      std::map<const State*, std::pair<bool, bool> > type;
      if (_GraphCheckInteractiveCycleDFS(*it, type)) {
        if (warnCycles) {
          warn("WARNING: Graph has an interactive cycle!");
          warn("The program may fail in any strange manner!!");
        }
        return true;
      }
    }
  }
  debug("Graph has no interactive cycle.");
  return false;
}

std::vector<State*>::size_type Graph::getStateNumber() const
{
  return states.size();
}

bool Graph::reachable(const State *const &fromState,
    const State *const &toState, const bool &onlyInteractive) const
{
  if (fromState == toState)
    return true;

  std::set<const State*> seen;
  std::set<const State*> toHandle;
  toHandle.insert(fromState);
  while (!toHandle.empty()) {
    const State* state = *toHandle.begin();
    toHandle.erase(toHandle.begin());
    for (std::vector<Transition*>::const_iterator trans =
        state->getTransitions()->begin();
        trans != state->getTransitions()->end();
        ++trans) {
      if (onlyInteractive && !(*trans)->isInteractive())
        continue;
      if ((*trans)->getTargetState() == toState)
        return true;
      if (seen.insert((*trans)->getTargetState()).second)
        toHandle.insert((*trans)->getTargetState());
    }
  }

  return false;
}

#ifdef DEBUG
std::vector<Transition*> Graph::getPath(const State *const &from,
    const State *const &to, const bool &onlyInteractive) const
{
  if (from == to)
    return std::vector<Transition*>();

  std::set<const State*> seen;
  std::deque< std::pair< std::vector<Transition*>, const State*> > toHandle;
  toHandle.push_back(std::make_pair(std::vector<Transition*>(), from));
  while (!toHandle.empty()) {
    for (std::vector<Transition*>::const_iterator trans =
        toHandle.front().second->getTransitions()->begin();
        trans != toHandle.front().second->getTransitions()->end();
        ++trans) {
      if (onlyInteractive && !(*trans)->isInteractive())
        continue;
      std::vector<Transition*> path = toHandle.front().first;
      path.push_back(*trans);
      if ((*trans)->getTargetState() == to)
        return path;
      if (seen.insert((*trans)->getTargetState()).second)
        toHandle.push_back(std::make_pair(path, (*trans)->getTargetState()));
    }
    toHandle.pop_front();
  }

  return std::vector<Transition*>();
}
#endif

void Graph::setAction(const std::string &newAction)
{
  theAction = newAction;
}

const std::string& Graph::getAction()
{
  return theAction;
}

void Graph::setCycleSearch(const bool &newCycleSearch)
{
  cycleSearch = newCycleSearch;
}

bool Graph::getCycleSearch()
{
  return cycleSearch;
}

const std::vector<State*>* Graph::getStates() const
{
  return &states;
}

std::vector<State*>* Graph::getStates()
{
  return &states;
}

void Graph::setComment(const std::string &newComment)
{
  comment.assign(newComment);
}

const std::string& Graph::getComment() const
{
  return comment;
}

void Graph::setInitialState(State* const &newInitialState)
{
  assert(checkStateInGraph(newInitialState));
  initialState = newInitialState;
}

State* Graph::getInitialState() const
{
  return initialState;
}

void Graph::registerInput(const std::string &format, GraphInput* handler)
{
  formatRegistry<GraphInput>::getInstance()->reg(format, handler);
}

void Graph::registerOutput(const std::string &format, GraphOutput* handler)
{
  formatRegistry<GraphOutput>::getInstance()->reg(format, handler);
}

std::vector<std::string> Graph::getInputFormats()
{
return formatRegistry<GraphInput>::getInstance()->getFormats();
}

std::vector<std::string> Graph::getOutputFormats()
{
return formatRegistry<GraphOutput>::getInstance()->getFormats();
}

void Graph::setReadyForExport(
    const bool &newReadyForExport)
{
  readyForExport = newReadyForExport;
}

bool Graph::isReadyForExport() const
{
  return readyForExport;
}

void Graph::setSearchForAbsorbingStates(
    const bool &newSearchForAbsorbingStates)
{
  searchForAbsorbingStates = newSearchForAbsorbingStates;
}

bool Graph::isSearchForAbsorbingStates()
{
  return searchForAbsorbingStates;
}

bool Graph::checkStateInGraph(State* const& aState) const
{
  for (std::vector<State*>::const_iterator state = states.begin();
      state != states.end(); ++state)
    if (*state == aState)
      return true;
  return false;
}

template <typename T>
void Graph::formatRegistry<T>::reg(const std::string &format, T* object)
{
  if (!registry.insert(std::make_pair(format, object)).second) {
    std::ostringstream error;
    error << "Format \"" << format << "\" already registered!";
    throw std::runtime_error(error.str().c_str());
  }
}

template <typename T>
T* Graph::formatRegistry<T>::get(const std::string &format) const
{
  typename std::map<std::string, T*>::const_iterator found =
    registry.find(format);
  if (found == registry.end())
    return NULL;
  return found->second;
}

template <typename T>
Graph::formatRegistry<T>* Graph::formatRegistry<T>::getInstance()
{
  static formatRegistry<T> instance;
  return &instance;
}

template <typename T>
Graph::formatRegistry<T>::formatRegistry()
{
  // private Constructor, hence can't be omitted
}

template <typename T>
Graph::formatRegistry<T>::~formatRegistry()
{
  assert(1);
  // nothing to do (this destructor shouldn't be called)
}

template <typename T>
std::vector<std::string> Graph::formatRegistry<T>::getFormats() const
{
  std::vector<std::string> formats;

  for (typename std::map<std::string, T*>::const_iterator it = registry.begin();
      it != registry.end(); ++it)
    formats.push_back(it->first);
  std::sort(formats.begin(), formats.end());

  return formats;
}

bool Graph::checkUniformity(const bool &showWarning, double* storeMaxOutgoingRate, double* storeMinOutgoingRate) const
{

  // save the minimal and maximal outgoing rate for assure uniformity
  double minOutgoingRate(0.0), maxOutgoingRate(0.0);
  bool firstOutgoingRate = true;

  for (std::vector<State*>::const_iterator state = states.begin();
      state != states.end(); ++state) {

    if ((*state)->getType() != State::MARKOV)
      continue;

    double outgoingRateHere = 0.0;

    for (std::vector<Transition*>::const_iterator trans =
        (*state)->getTransitions()->begin();
        trans != (*state)->getTransitions()->end();
        ++trans)
      outgoingRateHere += (*trans)->getRate();

    if (firstOutgoingRate) {
      firstOutgoingRate = false;
      minOutgoingRate = outgoingRateHere;
      maxOutgoingRate = outgoingRateHere;
    } else {
      if (outgoingRateHere < minOutgoingRate)
        minOutgoingRate = outgoingRateHere;
      if (outgoingRateHere > maxOutgoingRate)
        maxOutgoingRate = outgoingRateHere;
    }
  }

  if (storeMaxOutgoingRate != NULL)
    *storeMaxOutgoingRate = maxOutgoingRate;
  if (storeMinOutgoingRate != NULL)
    *storeMinOutgoingRate = minOutgoingRate;

  double diff = (minOutgoingRate - maxOutgoingRate) / maxOutgoingRate;
  if (diff < 0)
    diff *= -1;
  if (diff > uniformEpsilon) {
    if (showWarning) {
      warn("Warning: CTMDP is not uniform.");
      warn("Sum of outgoing rates min/max: %f/%f",
          minOutgoingRate, maxOutgoingRate);
    }
    return false;
  }

  return true;
}

void Graph::uniformize()
{
  double maxOutgoingRate;
  if (!checkUniformity(false, &maxOutgoingRate))
    uniformize(maxOutgoingRate);
}

void Graph::uniformize(const double maxOutgoingRate)
{

  unsigned int added = 0;

  for (std::vector<State*>::const_iterator state = states.begin();
      state != states.end(); ++state) {

    if ((*state)->getType() != State::MARKOV)
      continue;

    double myOutgoingRate = 0.0;
    for (std::vector<Transition*>::const_iterator trans =
        (*state)->getTransitions()->begin();
        trans != (*state)->getTransitions()->end();
        ++trans) {
      assert(!(*trans)->isInteractive());
      myOutgoingRate += (*trans)->getRate();
    }

    double diff = (maxOutgoingRate - myOutgoingRate) / maxOutgoingRate;

    if (diff > uniformEpsilon) {
      std::ostringstream label;
      label << "rate " << std::setprecision(10)
        << (maxOutgoingRate - myOutgoingRate);
      #ifdef DEBUG
      printf("Adding markov self-loop of %s to state %d.\n",
          label.str().c_str(), (*state)->getNumber());
      #endif
      (*state)->addTransition(new Transition(*state, label.str()));
      added++;
    } else if (diff < -uniformEpsilon) {
      warn("There is a State with higher outgoing rate than "
          "maxOutgoingRate: %f > %f.", myOutgoingRate, maxOutgoingRate);
    }
  }

  debug("Added %d markov self-loops.", added);
}

bool Graph::hasInternalNondeterminism() const
{
  for (std::vector<State*>::const_iterator state = states.begin();
      state != states.end(); ++state) {
    if ((*state)->getType() == State::MARKOV)
      continue;

    std::set<Label*> seenLabels;
    for (std::vector<Transition*>::const_iterator trans =
        (*state)->getTransitions()->begin();
        trans != (*state)->getTransitions()->end();
        ++trans)
      if (!seenLabels.insert((*trans)->getLabel()).second)
        return true;
  }

  return false;
}

void Graph::setColorDebug(char* newDebugColorString)
{
  unsigned int length = strlen(newDebugColorString);
  //delete[] colorDebug;
  colorDebug = new char[length+1];
  strncpy(colorDebug, newDebugColorString, length);
  colorDebug[length] = '\0';
}

void Graph::setColorWarning(char* newWarningColorString)
{
  unsigned int length = strlen(newWarningColorString);
  //delete[] colorWarning;
  colorWarning = new char[length+1];
  strncpy(colorWarning, newWarningColorString, length);
  colorWarning[length] = '\0';
}

void Graph::prepareForExport() {
  if (isReadyForExport())
    return;
  numberStates();
  sortStatesByNr();
}
