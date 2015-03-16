#ifndef __GRAPHOUTPUTETMCC_H
#define __GRAPHOUTPUTETMCC_H

#include <string>
#include "GraphOutput.h"
#include "Graph.h"

/** \brief Suitable for writing to ".tra" files in ETMCC format.
 *
 * The header of the "tra" format contains the number of States and Transitions
 * in the Graph. It looks as follows:
 * \verbatim
 *    STATES <nrOfStates>
 *    TRANSITIONS <nrOfTransitions>                               \endverbatim
 *
 * After the header, there will be a list of all Transitions, one Transition
 * per line. They are ordered by the number of the emanating State. The States
 * are numbered from one on (i.e. the last State has the number "nrOfStates").
 *
 * They are listed as follows:
 * \verbatim
 *    <prefix> <sourceStateNr> <targetStateNr> <rate> <suffix>    \endverbatim
 *
 * The prefix is "d" for interactive Transitions and "r" for markov
 * Transitions.
 * The rate of interactive Transitions is "0.0".
 * The suffix is "I" for interactive Transitions and "M" for markov
 * Transitions.
 *
 *
 * Beside the given filename, there will be a second file created, with the
 * same basename, but ".lab" as extension.
 * See GraphOutputLab for more details about this file.
 *
 * This class handles the "tra" format.
 */
class GraphOutputETMCC : public GraphOutput {
  public:
    /// See GraphOutput::writeToFile
    void writeToFile(Graph* graph, const std::string &format,
        const std::string &filename);

    /// Destructor
    ~GraphOutputETMCC();

  private:

    struct registerClass {
      registerClass() {
        Graph::registerOutput("tra", new GraphOutputETMCC);
      }
    };

    static registerClass registerObject;

    static const int stateNumberOffset = 1;

};

#endif

