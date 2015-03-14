#ifndef __GRAPH_H
#define __GRAPH_H

#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <stdio.h>

// Forward-Declarations
class State;
class Transition;
class Label;
class GraphInput;
class GraphOutput;

/// Encapsulates a whole Graph with all States and Transitions.
class Graph {

  public:
    /// Standard-Constructor
    Graph();

    /// Destructor
    ~Graph();

    /** \brief Factory for creating a Graph out of a file.
     *
     * The given format string is used to get the GraphInput object that
     * handles this format. The GraphInput class must have registered itself
     * using the registerInput() method.
     *
     * If the GraphInput object has been found, its GraphInput::readFromFile
     * method will be called.
     * If no GraphInput object registered the given format, an Exception will
     * be thrown.
     *
     * @param format   The format of the file to read.
     * @param filename The filename to read from.
     *
     * @return A new created Graph object that contains the states and
     *         transitions read from the file.
     *
     * @throw std::runtime_error if the given format has not been registered
     *                           by any GraphInput class
     */
    static Graph* read(const std::string &format, const std::string &filename)
      throw(std::runtime_error);

    /** \brief Write the Graph to a file.
     *
     * The given format string is used to get the GraphOutput object that
     * handles this format. The GraphOutput class mast have registered itself
     * using the registerOutput method.
     *
     * If the GraphOutput object has been found, its GraphOutput::writeToFile
     * method will be called.
     * If no GraphOutput object registered the given format, an Exception will
     * be thrown.
     *
     * @param format   The format of the file to write.
     * @param filename The filename to write to.
     *
     * @throw std::runtime_error if the given format has not been registered
     *                           by any GraphOutput class
     */
    void write(const std::string &format, const std::string &filename)
      throw(std::runtime_error);

    /** \brief Do the transformation from IMC to CTMDP
     *
     * The Graph is converted into a (strictly) alternating Graph as descriped
     * in the paper ctmdp.pdf.
     *
     * There is a restriction, that the Graph must not contain interactive
     * cycles.
     *
     * @param computeLabels Compute the labels to markov successors of interactive
     *                      States?
     *                      If false, all labels will be "DFS".
     */
    void transformImcToCtmdp(const bool &computeLabels = true);

    /** \brief Delete unreachable States
     *
     * All States, that are not reachable from the initial State, are deleted.
     */
    void deleteUnreachable();

    /** \brief Serially number all States
     *
     * Each States gets a unique Number between 0 and (states.size() - 1).
     * The initial State always gets 0, the next Numbers are assigned to the
     * interactive States, and then the markov States.
     */
    void numberStates() const;

    /// Sorts the States vector by the State numbers.
    void sortStatesByNr();

    /** \brief Store the label in the Graph object and return a pointer to it
     *
     * Because many labels are equal, it saves a lot of memory to store the
     * labels globally in the Graph object.
     * The Transitions should only contain pointers to the Labels
     * returned by this method.
     *
     * @param label The label to be searched and possibly inserted into the
     *              Graph's list of labels
     * @param quote If true, the given label is quoted (see Label::quote) before
     * 							it is processed
     */
    static Label* getLabelPtr(const std::string &label, bool quote = false);

    /** \brief Checks for an interactive cycle in the Graph.
     *
     * @param warnCycles If true, a warning is printed if a cycle is found.
     * @return true, if the Graph contains an interactive cycle, false
     *         otherwise.
     */
    bool checkInteractiveCycle(const bool &warnCycles = true) const;

    /** \brief State-to-State reachability check.
     *
     * @param fromState The State at which the path should start
     * @param toState   The State at which the path should end
     * @param onlyInteractive If true, only interactive Transitions are
     *                        followed
     * @return true, if a path from fromState to toState exists, false
     *         otherwise.
     */
    bool reachable(const State *const &fromState, const State *const &toState,
        const bool &onlyInteractive = false) const;

    #ifdef DEBUG
    std::vector<Transition*> getPath(const State *const &from,
        const State *const &to, const bool &onlyInteractive = false) const;
    #endif

    /// Returns the number of States in the Graph
    std::vector<State*>::size_type getStateNumber() const;

    /** \brief Set the label by that the 'interesting' States are marked.
     *
     * The marked States are the source States of the Transitions that are
     * labelled by the action string.
     *
     * Has to be called before the input file is read!
     */
    static void setAction(const std::string &newAction);

    /** \brief Get the label by that the 'interesting' States are marked.
     *
     * @return The string set by setAction(), an empty string if setAction()
     *         hasn't yet been called.
     */
    static const std::string& getAction();

    /** \brief Sets a comment string.
     *
     * Is used for some output formats.
     */
    void setComment(const std::string &newComment);

    /// Get the comment string.
    const std::string& getComment() const;

    /** \brief Set the initial State.
     *
     * Always ensure, that the given State is contained in the Graph!
     */
    void setInitialState(State* const& newInitialState);

    /// Get a pointer to the initial State.
    State* getInitialState() const;
    

    /** \brief Set the static attribute cycleSearch.
     *
     * Is used by the GraphInput-Classes to decide whether to search for
     * (interactive) cycles while reading the input file.
     * If set to no, you should ensure, that the read input files don't contain
     * interactive cycles!
     */
    static void setCycleSearch(const bool &newCycleSearch);

    /** \brief Get the static attribute cycleSearch.
     *
     * See setCycleSearch().
     */
    static bool getCycleSearch();

    /** \brief Set the static attribute searchForAbsorbingStates.
     *
     * See GraphOutputLab.
     */
    static void setSearchForAbsorbingStates(
        const bool &newSearchForAbsorbingStates);

    /** \brief Get the static attribute searchForAbsorbingStates.
     *
     * See setSearchForAbsorbingStates().
     */
    static bool isSearchForAbsorbingStates();

    /** \brief Set the static attribute readyForExport.
     *
     * If set to true, the method prepareForExport() immediately returns
     * if it is called.
     *
     * The intention of the readyForExport attribute is, that if the
     * Graph is exported to several formats, the preparation work should be
     * done only once (in prepareForExport()).
     * So if the graph is not modified any more, before you export it to many
     * formats, just call prepareForExport() and setReadyForExport(true).
     */
    void setReadyForExport(
        const bool &newReadyForExport);
    
    /** \brief Get the static attribute readyForExport.
     *
     * See setReadyForExport().
     */
    bool isReadyForExport() const;

    /** \brief Does some work to get the graph ready for export.
     * 
     * This method numbers all States (see numberStates()) and sorts them
     * (see sortStatesByNr()).
     */
    void prepareForExport();

    /** \brief Get the (constant) vector of all states.
     *
     * Used mainly by the GraphOutput objects, that need direct access to the
     * vector of all states.
     */
    const std::vector<State*>* getStates() const;

    /** \brief Get the vector of all states.
     *
     * Used mainly by the GraphInput objects, that need direct (write) access
     * to the vector of all states.
     */
    std::vector<State*>* getStates();

    /// Check if a specific State is contained in the Graph.
    bool checkStateInGraph(State* const& aState) const;

    /** \brief Check if the Graph is uniform
     *
     * Uniformity means, that for each markov state, the sum of outgoing 
     * Transitions must be the same (they may differ at least in the value
     * of uniformEpsilon).
     *
     * @param showWarning whether a waring should be printed if the graph
     *                    is not uniform
     * @param storeMaxOutgoingRate if set, then the maximal sum of outgoing
     *                             rates of the markov States is stored in the
     *                             referenced variable
     * @param storeMinOutgoingRate if set, then the minimal sum of outgoing
     *                             rates of the markov States is stored in the
     *                             referenced variable
     */
    bool checkUniformity(const bool &showWarning = true,
        double* storeMaxOutgoingRate = NULL,
        double* storeMinOutgoingRate = NULL) const;

    /** \brief Uniformizes the Graph.
     *
     * For the definition of uniformity, see checkUniformity().
     *
     * For a description, how uniformity is reached, see
     * uniformize(maxOutgoingRate).
     *
     * This function first calls checkUniformity() to decide whether there is
     * a need to uniformize and to know the maximal outgoing rate of a markov
     * State.
     * If you already know the maximal outgoing rate, then use the version,
     * that takes this as it's argument (uniformize(maxOutgoingRate)).
     */
    void uniformize();

    /** \brief Uniformize the Graph.
     *
     * This method uniformizes the Graph, such that all markov States have the
     * same sum of outgoing rates (maxOutgoingRate). This is done by adding a
     * markov self-loop Transition to States with lower sum of outgoing rates.
     *
     * @param maxOutgoingRate the maximal outgoing rate of a markov State.
     *                        If there is a higher outgoing rate found for some
     *                        markov State, a warning is printed.
     */
    void uniformize(const double maxOutgoingRate);

    /** \brief Check whether the Graph has internal nondeterminism.
     *
     * @return true, iff there is an interaktive State that has two outgoing
     *         (interactive) Transitions that have the same label, i.e.
     *         represent the same action
     */
    bool hasInternalNondeterminism() const;

    /** \brief Used by GraphInput-Classes to register themselves with certain
     *         input formats.
     *
     * If a GraphInput-Class can read more than one input format, this method
     * has to be called several times.
     *
     * @param format  A format, that the handler can handle.
     * @param handler A GraphInput object, that can handle the given input
     *                format.
     */
    static void registerInput(const std::string &format, GraphInput* handler);

    /** \brief Used by GraphOutput-Classes to register themselves with certain
     *         output formats.
     *
     * If a GraphOutput-Class can write more than one output format, this
     * method has to be calles several times.
     *
     * @param format  A format, that the handler can handle.
     * @param handler A GraphOutput object, that can handle the given output
     *                format
     */
    static void registerOutput(const std::string &format, GraphOutput* handler);

    /// Get all registered input formats.
    static std::vector<std::string> getInputFormats();

    /// Get all registered output formats.
    static std::vector<std::string> getOutputFormats();

    /** \brief Set the text color for debug messages.
     * Must be a string that let's the shell interpret the following
     * characters as colored. E.g. "\033[32m" for green.
     */
    static void setColorDebug(char* newDebugColorString);

    /** \brief Set the text color for warnings.
     * Must be a string that let's the shell interpret the following
     * characters as colored. E.g. "\033[31m" for red.
     */
    static void setColorWarning(char* newWarningColorString);


// the debug and warn methods aren't documented
#ifndef DOXYGEN
    // method that gets debug info and normally just writes them to STDOUT
    static void debug(const char* text) {
#ifndef NODEBUG
      printf("%s%s%s\n", colorDebug, text, colorReset);
#endif
    }

    template <typename T1>
    static void debug(const char* format, const T1 &p1) {
#ifndef NODEBUG
      printf(colorDebug); printf(format, p1); printf("%s\n", colorReset);
#endif
    }

    template <typename T1, typename T2>
    static void debug(const char* format, const T1 &p1, const T2 &p2) {
#ifndef NODEBUG
      printf(colorDebug); printf(format, p1, p2); printf("%s\n", colorReset);
#endif
    }

    template <typename T1, typename T2, typename T3>
    static void debug(const char* format, const T1 &p1, const T2 &p2, const T3 &p3) {
#ifndef NODEBUG
      printf(colorDebug); printf(format, p1, p2, p3); printf("%s\n", colorReset);
#endif
    }

    // same in red color
    static void warn(const char* text) {
#ifndef NOWARN
      printf("%s%s%s\n", colorWarning, text, colorReset);
#endif
    }

    template <typename T1>
    static void warn(const char* format, const T1 &p1) {
#ifndef NOWARN
      printf(colorWarning); printf(format, p1); printf("%s\n", colorReset);
#endif
    }

    template <typename T1, typename T2>
    static void warn(const char* format, const T1 &p1, const T2 &p2) {
#ifndef NOWARN
      printf(colorWarning); printf(format, p1, p2); printf("%s\n", colorReset);
#endif
    }

    template <typename T1, typename T2, typename T3>
    static void warn(const char* format, const T1 &p1, const T2 &p2, const T3 &p3) {
#ifndef NOWARN
      printf(colorWarning); printf(format, p1, p2, p3); printf("%s\n", colorReset);
#endif
    }
#endif

  private:

    template <typename T1>
      class formatRegistry {
        public:
          void reg(const std::string &format, T1* object);
          T1* get(const std::string &format) const;
          static formatRegistry* getInstance();
          std::vector<std::string> getFormats() const;
        private:
          formatRegistry();
          ~formatRegistry();
          std::map<std::string, T1*> registry;
      };

    static Label* getLabelPtr(const std::string *label);

    // vector of all States
    std::vector<State*> states;

    // map of Labels
    // the Transitions only know pointers to this Labels, because many
    // labels are equal
    static std::map<std::string, Label*> labels;

    // pointer to initial state
    State* initialState;

    // a comment string of the Graph
    std::string comment;

    // label, that marks the interesting states
    static std::string theAction;

    // options for reading the input files
    static bool cycleSearch;

    // if this is set, the graph shouldn't change any more
    bool readyForExport;

    // option for writing the output file
    static bool searchForAbsorbingStates;

    // different colors for debug/warning output
    static char* colorDebug;
    static char* colorWarning;
    static char* colorReset;

    // the maximal value that the sums of outgoing transitions may differ
    static const double uniformEpsilon;

};

#endif

