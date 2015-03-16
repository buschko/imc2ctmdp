#ifndef __GRAPHOUTPUT_H
#define __GRAPHOUTPUT_H

#include <string>

// forward-declaration
class Graph;

/// Responsible for exporting a Graph object to a file.
class GraphOutput {
  public:
    /** \brief Method for writing a Graph object to a file.
     *
     * @param graph The Graph object to export.
     * @param format The format of the output file (typically the extension
     *               of the filename). May be used for internal determinations.
     * @param filename The filename to export the Graph to.
     */
    virtual void writeToFile(Graph* graph, const std::string &format,
        const std::string &filename) = 0;

    /// Destructor
    virtual ~GraphOutput();

  private:

};

#endif

