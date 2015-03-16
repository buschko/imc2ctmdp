#ifndef __GRAPHINPUTBCG_H
#define __GRAPHINPUTBCG_H

#include <string>
#include "GraphInput.h"
#include "Graph.h"

/** \brief Suitable for reading a Graph from a BCG file
 *
 * This GraphInput uses the BCG library from the CADP project (see also
 * http://www.inrialpes.fr/vasy/cadp/man/bcg.html).
 * This file contains all States, Transitions and Labels.
 *
 */
class GraphInputBCG : public GraphInput {
  public:
    /// see GraphInput::readFromFile
    Graph* readFromFile(const std::string &format,
        const std::string &filename);

    /// Destructor
    ~GraphInputBCG();

  private:
    
    struct registerClass {
      registerClass() {
        Graph::registerInput("bcg", new GraphInputBCG);
      }
    };

    static registerClass registerObject;

};

#endif

