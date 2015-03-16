#ifndef __GRAPHINPUT_H
#define __GRAPHINPUT_H

#include <string>

// forward-declaration
class Graph;

/// Responsible for importing a Graph object from a file.
class GraphInput {
  public:
    /** \brief Method to read a Graph from a File.
     *
     * A new Graph-instance is created and read from the given filename.
     *
     * The returned Graph object should be complete, i.e. contain all States,
     * Transitions and Labels.
     *
     * @param format The format (typically the file extension) of the file to
     *               read from. May be used for internal determinations.
     * @param filename The filename to read the Graph from.
     *
     * @return A Pointer to the created Graph object.
     */
    virtual Graph* readFromFile(const std::string &format,
        const std::string &filename) = 0;

    /// Destructor
    virtual ~GraphInput();

  private:

};

#endif

