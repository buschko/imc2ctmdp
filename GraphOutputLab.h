#ifndef __GRAPHOUTPUTLAB_H
#define __GRAPHOUTPUTLAB_H

#include <string>
#include "GraphOutput.h"
#include "Graph.h"

/** \brief Suitable for writing to a ".lab" file.
 *
 * The written file contains all state labels.
 * The format can directly be read by MRMC.
 *
 * There are basically two state labels in this file.
 * The "reach" label marks all interesting States, so that MRMC can check
 * the propability to reach one of these States.
 *
 * If Graph::getSearchForAbsorbingStates yields true, then a second label,
 * "absorbing", will be created, that marks all States, that have either no
 * outgoing Transition, or only has one Transition, that just leads back to
 * the same State.
 * If no such States exist, the label will not be deklared.
 *
 * This file has the header
 * \verbatim
 *    #DECLARATION
 *    reach
 *    absorbing
 *    #END                                                        \endverbatim
 * Where absorbing is only introduces, if there is at least one absorbing State.
 *
 * The following lines have the form
 * \verbatim
 *    <stateNr> <label> [<label>]                                 \endverbatim
 * Where label may be one of "reach" and "absorbing".
 *
 * This class handles the "lab" format.
 */
class GraphOutputLab : public GraphOutput {
  public:
    /// See GraphOutput::writeToFile
    void writeToFile(Graph* graph, const std::string &format,
        const std::string &filename);

    /// Destructor
    ~GraphOutputLab();

  private:

    struct registerClass {
      registerClass() {
        GraphOutput* o = new GraphOutputLab();
        Graph::registerOutput("lab", o);
      }
    };

    static registerClass registerObject;

    static const unsigned short stateNumberOffset = 1;

    static const char* reachLabel;
    static const char* absorbingLabel;

};

#endif

