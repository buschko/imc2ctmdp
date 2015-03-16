#ifndef __GRAPHOUTPUTBCG_H
#define __GRAPHOUTPUTBCG_H

#include <string>
#include "GraphOutput.h"
#include "Graph.h"

/** \brief Suitable for writing to BCG file
 *
 * This class uses the BCG library from CADP (see
 * http://www.inrialpes.fr/vasy/cadp/man/bcg.html) to write the whole
 * Graph with all information needed to one binary file.
 *
 */
class GraphOutputBCG : public GraphOutput {
  public:
    /// See GraphOutput::writeToFile
    void writeToFile(Graph* graph, const std::string &format,
        const std::string &filename);

    /// Destructor
    ~GraphOutputBCG();

  private:

    struct registerClass {
      registerClass() {
        Graph::registerOutput("bcg", new GraphOutputBCG);
      }
    };

    static registerClass registerObject;

};

#endif

