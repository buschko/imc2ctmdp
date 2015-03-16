#ifndef __GRAPHOUTPUTMARKED_H
#define __GRAPHOUTPUTMARKED_H

#include <string>
#include "GraphOutput.h"
#include "Graph.h"

/** \brief Suitable for writing ".marked" files.
 *
 * The "marked" format is very easy: It just contains a list of all marked
 * States (one number per line).
 *
 * The States are numbered from zero on.
 *
 * This Class only handles the "marked" format.
 */
class GraphOutputMarked : public GraphOutput {
  public:
    /// See GraphOutput::writeToFile
    void writeToFile(Graph* graph, const std::string &format,
        const std::string &filename);

    /// Destructor
    ~GraphOutputMarked();

  private:

    struct registerClass {
      registerClass() {
        Graph::registerOutput("marked", new GraphOutputMarked);
      }
    };

    static registerClass registerObject;

};

#endif

