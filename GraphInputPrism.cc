#include "GraphInputPrism.h"
#include <string>
#include <fstream>
#include <sstream>
#include <cassert>
#include <map>
#include "Graph.h"
#include "State.h"
#include "Transition.h"

bool streamEmpty(std::istream&);

Graph* GraphInputPrism::readFromFile(const std::string &,
    const std::string &filename)
{
  // compute the filenames of the transitions / labels file
  std::string transFilename, labelsFilename;
  if ((filename.size() >= 6) &&
      (filename.substr(filename.size() - 6) == ".prism")) {
    transFilename = filename.substr(0, filename.size() - 6) + ".trans";
    labelsFilename = filename.substr(0, filename.size() - 6) + ".labels";
  } else if ((filename.size() >= 3) &&
      ((filename.substr(filename.size() - 3) == ".sm") ||
       (filename.substr(filename.size() - 3) == ".nm") ||
       (filename.substr(filename.size() - 3) == ".pm"))) {
    transFilename = filename.substr(0, filename.size() - 3) + ".trans";
    labelsFilename = filename.substr(0, filename.size() - 3) + ".labels";
  } else {
    transFilename = filename + ".trans";
    labelsFilename = filename + ".labels";
  }

  // create new Graph object
  Graph* graph = new Graph();

  // the three files
  std::ifstream prismFile, transFile, labelsFile;

  // open the label file and read the number of the initial state
  labelsFile.open(labelsFilename.c_str(), std::ios::in);

  if (!labelsFile) {
    graph->warn("Error opening labels file \"%s\"", labelsFilename.c_str());
    delete graph;
    return NULL;
  }

  std::string line;

  // read first line and get the number of the "init" attribute
  getline(labelsFile, line);

  unsigned int nrInitAttribute(0);
  {
    std::string attribute;
    bool ok(true);

    // read the items of the form <nr>="<label>"
    // and search for the label "init"
    std::string::iterator nextChar = line.begin();
    while (ok && (attribute != "init")) {
      nrInitAttribute = 0;
      attribute = "";
      while ((nextChar != line.end()) && isdigit(*nextChar)) {
        nrInitAttribute = 10 * nrInitAttribute + *nextChar - '0';
        ++nextChar;
      }
      ok &= (nextChar != line.end()) && (*nextChar++ == '=');
      ok &= (nextChar != line.end()) && (*nextChar++ == '"');
      while (ok && (nextChar != line.end()) && (*nextChar != '"'))
        attribute.append(1, *nextChar++);
      ok &= (nextChar != line.end()) && (*nextChar++ == '"');
    }

    if (!ok) {
      graph->warn("Error reading first line of labels file: %s", line.c_str());
      delete graph;
      return NULL;
    }

    // nrInitAttribute is now set correctly
  }

  // now read the other lines of the labels file and search for nrInitAttribute

  unsigned int initialStateNr(0);
  bool foundInitialStateNr(false);
  while (!labelsFile.eof()) {
    getline(labelsFile, line);
    if (labelsFile.eof() && line.empty())
      break;
    std::string::size_type posColon = line.find(':');
    bool ok = (posColon != line.npos) && (posColon != line.size() - 1);
    if (ok) {
      std::istringstream stateStream(line.substr(0, posColon)),
        nrStream(line.substr(posColon+1));
      unsigned int stateNr, attributeNr;
      stateStream >> stateNr;
      nrStream >> attributeNr;
      ok &= stateStream && nrStream && streamEmpty(stateStream) &&
        streamEmpty(nrStream);
      if (attributeNr == nrInitAttribute) {
        if (!foundInitialStateNr) {
          initialStateNr = stateNr;
          foundInitialStateNr = true;
        } else {
          graph->warn("Error: found second initial state in labels file, "
              "line: %s", line.c_str());
          labelsFile.close();
          delete graph;
          return NULL;
        }
      }
    }
    if (!ok) {
      graph->warn("Error in line of labels file: %s", line.c_str());
      labelsFile.close();
      delete graph;
      return NULL;
    }
  }

  // ready reading, close labelsFile
  labelsFile.close();

  if (!foundInitialStateNr) {
    graph->warn("Error: Couldn't find the initial State if states file");
    delete graph;
    return NULL;
  }


  // first file is read, now read the prism file and search for action labels

  prismFile.open(filename.c_str(), std::ios::in);
  if (!prismFile) {
    graph->warn("Error opening prism file \"%s\"", filename.c_str());
    delete graph;
    return NULL;
  }


  // actionRate is the rate of the marked action
  double actionRate = -1e30+1; // a number that shouldn't occure
  
  // the rateLabels map contains a mapping from rates to Labels
  // for each interactive transition (remember that in the PRISM file,
  // interactive Transitions are introduced by special constants)
  std::map<double, Label*> rateLabels;

  unsigned int nrReadActions(0);

  while (!prismFile.eof()) {
    getline(prismFile, line);

    // the line is parsed as follows:
    // Search of the character '='. Before that character, there must be the
    // words "const" and "int" or "double", follow by an identifier.
    // Behind the '=', there must be a double, followed by a semicolon.
    // And behind that semicolon, there must be "// Action", follow by the
    // action label

    std::string::size_type posEquals = line.find('=');
    if ((posEquals == line.npos) || (posEquals == line.size() - 1))
      continue;

    std::istringstream lineStream(line.substr(0, posEquals));
    std::string word;
    lineStream >> word;
    if (!lineStream || (word != "const"))
      continue;
    lineStream >> word;
    if (!lineStream || ((word != "int") && (word != "double")))
      continue;
    lineStream >> word;
    if (!streamEmpty(lineStream))
      continue;

    std::string::size_type posSemicolon = line.find(';', posEquals);
    if ((posSemicolon == line.npos) || (posSemicolon == line.size() - 1))
      continue;
    lineStream.clear();
    lineStream.str(line.substr(posEquals + 1, posSemicolon - posEquals - 1));
    double rate;
    lineStream >> rate;
    if (!lineStream || !streamEmpty(lineStream))
      continue;

    std::string::size_type posFirstSlash = line.find_first_not_of(" \t",
        posSemicolon + 1);
    if ((posFirstSlash == line.npos) || (posFirstSlash >= line.size() - 2) ||
        (line[posFirstSlash] != '/') || (line[posFirstSlash+1] != '/'))
      continue;

    std::string::size_type posFirstDoubleQuote = line.find('"', posFirstSlash);
    if ((posFirstDoubleQuote == line.npos) ||
        (posFirstDoubleQuote >= line.size() - 2))
      continue;

    lineStream.clear();
    lineStream.str(line.substr(posFirstSlash + 2,
          posFirstDoubleQuote - posFirstSlash - 2));
    lineStream >> word;
    if (!lineStream || !streamEmpty(lineStream) || (word != "Action"))
      continue;

    std::string::size_type posSecondDoubleQuote = line.find('"',
        posFirstDoubleQuote + 1);
    if (posSecondDoubleQuote == line.npos)
      continue;

    std::string action = line.substr(posFirstDoubleQuote + 1,
        posSecondDoubleQuote - posFirstDoubleQuote - 1);

    // now we are ready: we know the number and the action label, and now
    // have to write them in the map
    rateLabels[rate] = graph->getLabelPtr(action, true);

    if (action == graph->getAction())
      actionRate = rate;

    ++nrReadActions;
  }

  graph->debug("Read %d actions from prism file", nrReadActions);

  prismFile.close();


  // prismFile is ready
  // now read the transitions file


  transFile.open(transFilename.c_str(), std::ios::in);
  if (!transFile) {
    graph->warn("Error opening transitions file \"%s\"", transFilename.c_str());
    delete graph;
    return NULL;
  }

  // read first line from transitions file
  // this line contains 2 numbers (#states, #transitions)
  getline(transFile, line);
  // parse the line
  std::istringstream lineStream(line);
  unsigned int noStates, noTransitions;
  lineStream >> noStates >> noTransitions;
  if (!lineStream || !streamEmpty(lineStream)) {
    graph->warn("Error reading first line of transitions file (expected two "
        "numbers, found \"%s\"", line.c_str());
    transFile.close();
    delete graph;
    return NULL;
  }
  
  // output some debug information
  graph->debug("########################################");
  graph->debug("INPUT statistics:");
  graph->debug("  %-23s%15d", "Number of states:", noStates);
  graph->debug("  %-23s%15d", "Number of transitions:", noTransitions);
  graph->debug("########################################");

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
  assert(initialStateNr < noStates);
  graph->setInitialState(states[initialStateNr]);

  // count the number of removed and read transitions
  unsigned int removed(0), read(0);

  while (!transFile.eof()) {
    getline(transFile, line);

    if (transFile.eof() && line.empty())
      break;

    unsigned int sourceStateNr, targetStateNr;
    double rate;
    std::istringstream lineStream(line);
    lineStream >> sourceStateNr >> targetStateNr >> rate;
    if (!lineStream || !streamEmpty(lineStream)) {
      graph->warn("Error reading transition file, line nr. %d, expected three "
          "numbers but found \"%s\"", read+1, line.c_str());
      transFile.close();
      delete graph;
      return NULL;
    }

    ++read;
    
    // some assertions for the read values
    assert(sourceStateNr < noStates);
    assert(targetStateNr < noStates);

    State *&sourceState = states[sourceStateNr],
          *&targetState = states[targetStateNr];

    // if the rate is the action rate, just mark the State
    if (rate == actionRate) {
      states[sourceStateNr]->setMark(true);
      continue;
    }

    // get the label for that rate
    std::map<double, Label*>::const_iterator label =
      rateLabels.find(rate);
    // create one if it doesn't exist
    if (label == rateLabels.end()) {
      std::ostringstream labelString;
      labelString << "rate " << rate;
      label = rateLabels.insert(
          std::make_pair(rate, graph->getLabelPtr(labelString.str(), true))).first;
    }

    Transition* newTransition = new Transition(targetState, label->second);

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

  }

  if (read != noTransitions)
    graph->warn("Warning: Number of read transitions differs from specification"
        " in the first line. Expected: %d, read: %d", noTransitions, read);

  if (removed > 0)
    graph->warn("%d transitions removed because they would have caused interactive cycles", removed);

  // close transitions file
  transFile.close();

  return graph;

}

GraphInputPrism::~GraphInputPrism()
{
  // nothing to do here
}

GraphInputPrism::registerClass GraphInputPrism::registerObject;


bool streamEmpty(std::istream &toTest)
{
  char test('\0');
  toTest >> test;
  return toTest.eof();
}

