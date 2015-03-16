#ifndef __GRAPHOUTPUTCTMDP_H
#define __GRAPHOUTPUTCTMDP_H

#include <string>
#include "GraphOutput.h"
#include "Graph.h"

/** \brief Suitable for writing to a ".ctmdp[i]" file.
 *
 * The header of the "ctmdp" and "ctmdpi" format contains the number of
 * States in the Graph and a list of all action labels. It looks as follows:
 * \verbatim
 *    #STATES <nrOfStates>
 *    #DECLARATION
 *    <first action label>
 *    <second action label>
 *    ...
 *    <n-th action label>
 *    #END                                                        \endverbatim
 *
 * The actionLabel of \f$ \tau \f$ ("tau") Transitions is "i".
 *
 * After the header, there will be a list of all Transitions, one Transition
 * per line. They are ordered by the number of the emanating State. The States
 * are numbered from one on (i.e. the last State has the number "nrOfStates").
 *
 * In the "ctmdp" format, they look as follows:
 * \verbatim
 *    <sourceStateNr> <targetStateNr> <actionLabel> <propability> \endverbatim
 *
 * In the "ctmdpi" format, they are grouped by action labels and look as
 * follows:
 * \verbatim
 *    <sourceStateNr> <actionLabel>
 *    * <targetStateNr> <propability>
 *    * <targetStateNr> <propability>
 *    ...                                                         \endverbatim
 *
 *
 * Beside the given filename, there will be a second file created, with the
 * same basename, but ".lab" as extension.
 * See GraphOutputLab for more details about this file.
 *
 * This class handles the "ctmdp" and the "ctmdpi" formats.
 */
class GraphOutputCtmdp : public GraphOutput {
  public:
    /// See GraphOutput::writeToFile
    void writeToFile(Graph* graph, const std::string &format,
        const std::string &filename);

    /// Destructor
    ~GraphOutputCtmdp();

  private:

    struct registerClass {
      registerClass() {
        GraphOutput* o = new GraphOutputCtmdp;
        Graph::registerOutput("ctmdp", o);
        Graph::registerOutput("ctmdpi", o);
      }
    };

    static registerClass registerObject;

    static const unsigned short stateNumberOffset = 1;

};

#endif

