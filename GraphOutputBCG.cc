#include "GraphOutputBCG.h"
#include <string>
#include <bcg_user.h>
#include "State.h"
#include "Transition.h"

void GraphOutputBCG::writeToFile(Graph* graph, const std::string &,
        const std::string &filename)
{
  // get the Graph ready for export
  graph->prepareForExport();

  graph->debug("Writing BCG file");
  graph->debug("OUTPUT %s", filename.c_str());

  // convert filename to C-String (must be non-const)
  char* filename_char = new char[filename.length()+1];
  filename.copy(filename_char, filename.length());
  filename_char[filename.length()] = '\0';

  // convert comment to C-String (must be non-const)
  char* comment_char = new char[graph->getComment().length()+1];
  graph->getComment().copy(comment_char, graph->getComment().length());
  comment_char[graph->getComment().length()] = '\0';

  BCG_INIT();
  BCG_IO_WRITE_BCG_PARSING (0); // don't try to parse labels

  BCG_IO_WRITE_BCG_BEGIN(filename_char, // filename
      0,            // number of initial state
      2,            // format: Edges are written sequential (monotonously)
      comment_char, // the comment
      0             // no display
  );

  delete [] filename_char;
  delete [] comment_char;

  // iterate over all states
  std::vector<State*> &states = *graph->getStates();
  for (std::vector<State*>::const_iterator state = states.begin();
      state != states.end(); ++state)
    // write all transitions emanating this state
    for (std::vector<Transition*>::const_iterator trans =
        (*state)->getTransitions()->begin();
        trans != (*state)->getTransitions()->end(); ++trans)
      // [label must be non-const (Bug in BCG! grr...)]
      // if an interactive State is marked, then all outgoing Transitions
      // are labelled by 'theAction'
      BCG_IO_WRITE_BCG_EDGE((*state)->getNumber(),
          const_cast<char*>(
            ((*state)->getMark() && ((*state)->getType() == State::INTERACTIVE))
              ? graph->getAction().c_str() : (*trans)->getLabel()->str().c_str()),
          (*trans)->getTargetState()->getNumber());

  BCG_IO_WRITE_BCG_END ();
}

GraphOutputBCG::~GraphOutputBCG()
{
  // nothing to do
}

GraphOutputBCG::registerClass GraphOutputBCG::registerObject;

