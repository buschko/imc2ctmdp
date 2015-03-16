#include "imc2mrmc.h"
#include <string>
#include "../Graph.h"
#include "../State.h"
#include "../Transition.h"
#include "../GraphInputBCG.h"
#include "../GraphInputPrism.h"


std::pair<std::string, std::string> getFormat(const std::string &filename,
    const std::string &default_filename);
mdp_sparse* readSparse(Graph*);
labelling*  readLabelling(Graph*);

char* markedToken = "reach";

void registerInputs() {
  // for C, we have create each GraphInput object, so that it registeres itself
  static bool registered = false;
  if (registered)
    return;

  GraphInputBCG* inputBCG = new GraphInputBCG;
  delete inputBCG;
  GraphInputPrism* inputPrism = new GraphInputPrism;
  delete inputPrism;

  registered = true;
}

extern "C" {

  void read_imc(const char filename[], const char actionLabel[],
      mdp_sparse** sparse, labelling** labels) {

    // in C, we have to register the input formats manually
    registerInputs();
  
    try {
  
      // set options
      Graph::setAction(std::string(actionLabel));
  
      std::string inputFilename(filename);

      std::pair<std::string, std::string> inputFormat = getFormat(inputFilename,
          std::string());
      if (inputFormat.first.empty()) {
        inputFilename.append(".bcg");
        inputFormat.first = "bcg";
        inputFormat.second = inputFilename;
      }
      Graph* graph = Graph::read(inputFormat.first, inputFormat.second);
      if (!graph) {
        graph->warn("Error reading input file, exiting.");
        exit(-1);
      }
  
      graph->debug("Transforming IMC to CTMDP");
      graph->transformImcToCtmdp();
  
      graph->debug("Exporting to MRMC format");
  
      graph->numberStates();
      graph->sortStatesByNr();
  
      *sparse = readSparse(graph);
  
      *labels = readLabelling(graph);
  
      // delete the Graph
      delete graph;
  
    }
    catch (std::exception &e) {
      printf("\033[31m\033[47mAn exception occured: %s\033[0m\n", e.what());
    }
    catch (...) {
      printf("\033[31m\033[47mAn UNKNOWN EXCEPTION occured!!\033[0m\n");
    }
  }

}

std::pair<std::string, std::string> getFormat(const std::string &filename,
    const std::string &default_filename)
{
  std::string::size_type pos = filename.find(':');
  if (pos == filename.size() - 1)
    return std::make_pair(filename.substr(0, pos),
        default_filename + (filename.substr(0,pos) == "bcg" ? "_ctmdp" : "") +
        "." + filename.substr(0, pos));
  if (pos != filename.npos)
    return std::make_pair(filename.substr(0, pos), filename.substr(pos+1));
  pos = filename.rfind('.');
  if (pos != filename.npos)
    return std::make_pair(filename.substr(pos+1), filename);

  return std::make_pair(std::string(), filename);
}



mdp_sparse* readSparse(Graph* graph) {

  std::vector<State*>* states = graph->getStates();

  // get number of interactive States
  int nrInteractiveStates(0);
  for (std::vector<State*>::const_iterator state = states->begin();
      state != states->end(); ++state) {
    if ((*state)->getType() != State::INTERACTIVE)
      break;
    ++nrInteractiveStates;
  }

  
  // Map of all actions and their internal number
  int nextActionNumber = 0;
  std::map<std::string, int> allActions;

  // search for all action labels
  int toDo = nrInteractiveStates;
  for (std::vector<State*>::const_iterator state = states->begin();
      toDo != 0; ++state, --toDo) {

    for (std::vector<Transition*>::const_iterator trans =
        (*state)->getTransitions()->begin();
        trans != (*state)->getTransitions()->end();
        ++trans) {
      const std::string action = (*trans)->getLabel()->str();
      std::map<std::string, int>::iterator found = allActions.find(action);
      if (found == allActions.end())
        allActions.insert(std::make_pair(action, nextActionNumber++));
    }
  }

  // create MRMC labelset
  mdp_labelset* labels = mdp_labelset_new(nextActionNumber);
  {
    std::vector<std::string> actionsByNr(nextActionNumber, std::string());
    for (std::map<std::string, int>::const_iterator it = allActions.begin();
        it != allActions.end(); ++it)
      actionsByNr[it->second] = it->first;

    for (std::vector<std::string>::const_iterator it = actionsByNr.begin();
        it != actionsByNr.end(); ++it)
      mdp_labelset_add(labels, it->c_str());
  }

  // create mdp sparse structure
  mdp_sparse* sparse = (mdp_sparse*)malloc(sizeof(mdp_sparse));
  sparse->num_states = nrInteractiveStates;
  sparse->val = (mdp_values*)calloc(nrInteractiveStates, sizeof(mdp_values));
  sparse->label = labels;

  // save the minimal and maximal outgoing rate for assure uniformity
  double minOutgoingRate(0.0), maxOutgoingRate(0.0);
  bool firstOutgoingRate = true;

  // and now the main work...
  toDo = nrInteractiveStates;
  for (std::vector<State*>::const_iterator state = states->begin();
      toDo > 0; ++state, --toDo) {

    // all actions available from this State, and their probability to get
    // to another State
    // a map from action numbers (label numbers) to a map from states to
    // propabilities
    std::map<int, std::map<int, double> > actions;

    std::vector<Transition*>* transitions = (*state)->getTransitions();

    for (std::vector<Transition*>::const_iterator trans = transitions->begin();
        trans != transitions->end(); ++trans) {

      State* target = (*trans)->getTargetState();
      std::string label = (*trans)->getLabel()->str();
      int actionNr = allActions[label];


      for (std::vector<Transition*>::const_iterator mTrans =
          target->getTransitions()->begin();
          mTrans != target->getTransitions()->end();
          ++mTrans) {

        // add the found transition to the vector
        actions[actionNr][(*mTrans)->getTargetState()->getNumber()] +=
          (*mTrans)->getRate();
      }
    }

    // dump the transitions/actions vector
    int stateNr = (*state)->getNumber();

    sparse->val[stateNr].col =
      (int**)calloc(nextActionNumber, sizeof(int *));
    sparse->val[stateNr].val =
      (double**)calloc(nextActionNumber, sizeof(double *));
    sparse->val[stateNr].val_sizes =
      (int*)calloc(nextActionNumber, sizeof(int));
    sparse->val[stateNr].size = 0;

    // now save the found transitions in the MRMC data structure
    int lastActionNr = 0;
    for (std::map<int, std::map<int, double> >::const_iterator it =
        actions.begin(); it != actions.end(); ++it) {

      int actionNr = it->first;
      while (++lastActionNr < actionNr) {
        sparse->val[stateNr].col[lastActionNr] = (int*)calloc(0, sizeof(int));
        sparse->val[stateNr].val[lastActionNr] =
          (double*)calloc(0, sizeof(double));
        sparse->val[stateNr].val_sizes[lastActionNr] = 0;
      }

      const std::map<int, double> &targets = it->second;

      // the sum of outgoing transitions of the actual action laben
      double outgoingRate(0.0);

      int*    cols = (int*)calloc(targets.size(), sizeof(int));
      double* vals = (double*)calloc(targets.size(), sizeof(double));

      sparse->val[stateNr].val_sizes[actionNr] = targets.size();
      sparse->val[stateNr].size += targets.size();

      int transNr = 0;

      for (std::map<int, double>::const_iterator trans =
          targets.begin(); trans != targets.end(); ++trans, ++transNr) {
        cols[transNr] = trans->first;
        vals[transNr] = trans->second;
        outgoingRate += trans->second;
      }

      sparse->val[stateNr].col[actionNr] = cols;
      sparse->val[stateNr].val[actionNr] = vals;

      if (firstOutgoingRate) {
        firstOutgoingRate = false;
        minOutgoingRate = outgoingRate;
        maxOutgoingRate = outgoingRate;
      } else {
        if (outgoingRate < minOutgoingRate)
          minOutgoingRate = outgoingRate;
        if (outgoingRate > maxOutgoingRate)
          maxOutgoingRate = outgoingRate;
      }

    }

    while ((unsigned int)++lastActionNr < actions.size()) {
      sparse->val[stateNr].col[lastActionNr] = (int*)calloc(0, sizeof(int));
      sparse->val[stateNr].val[lastActionNr] =
        (double*)calloc(0, sizeof(double));
      sparse->val[stateNr].val_sizes[lastActionNr] = 0;
    }

  }

  double outgoingRateRatio = maxOutgoingRate == 0.0 
    ? 0.0 
    : minOutgoingRate / maxOutgoingRate;
  if ((outgoingRateRatio < 0.99) || (outgoingRateRatio > 1.01)) {
    graph->warn("Warning: CTMDP is not uniform.");
    graph->warn("Sum of outgoing rates min/max: %f/%f",
        minOutgoingRate, maxOutgoingRate);
  } else
    graph->debug("Sum of outgoing rates min/max: %f/%f",
        minOutgoingRate, maxOutgoingRate);

  return sparse;
}


labelling*  readLabelling(Graph* graph) {
  labelling* labels = get_new_label(1, graph->getStateNumber());
  add_label(labels, markedToken);

  std::vector<State*>* states = graph->getStates();

  for (std::vector<State*>::const_iterator it = states->begin();
      it != states->end(); ++it)
    if ((*it)->getMark())
      set_label_bit(labels, markedToken, (*it)->getNumber());

  return labels;

}

