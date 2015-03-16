#ifndef __GRAPHINPUTPRISM_H
#define __GRAPHINPUTPRISM_H

#include <string>
#include "GraphInput.h"
#include "Graph.h"

/** \brief Suitable for reading a Graph from a PRISM file
 *
 * This class reads a graph from three files (see readFromFile()): The prism
 * file, the transitions file and the labels file.
 *
 * The prism file is searched for special comments, that mark interactive
 * transitions. These comments must have the form
 * \verbatim
 * const int action1 = 10001; // Action "Interact 1"            \endverbatim
 * (you can also use double values)
 * In words: The comment must be behind the defined prism variable, and must
 * begin with the key word "Action". Behind the keyword, the name for the
 * interactive Transition is given (in double-quotes).
 *
 * The labels file is needed for finding the initial state.
 *
 * The "main file" is the transition file, because it contains all transitions
 * from one state to another. While reading this file, all interactive
 * transitions are saved with their name.
 */
class GraphInputPrism : public GraphInput {
  public:
    /** \brief Create a Graph from a file
     *
     * For general explanations see GraphInput::readFromFile.
     *
     * There must also exist two files with the same basename as
     * the given filename, but with the extensions ".trans" and ".labels".
     * These files can be created using the prism software and the switches 
     * "-exporttrans" and "-exportlabels".
     */
    Graph* readFromFile(const std::string &format,
        const std::string &filename);

    /// Destructor
    ~GraphInputPrism();

  private:
    
    struct registerClass {
      registerClass() {
        GraphInput* o = new GraphInputPrism;
        Graph::registerInput("prism", o);
        Graph::registerInput("nm", o);
        Graph::registerInput("sm", o);
        Graph::registerInput("pm", o);
      }
    };

    static registerClass registerObject;

};

#endif

