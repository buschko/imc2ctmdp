#ifndef __STATE_H
#define __STATE_H

#include <vector>
#include <string>

// forward-declaration
class Transition;

/** \brief Represents a single State
 *
 * This class encapsulates one State, together with it's outgoing Transitions.
 *
 */
class State {
  public:

    /// The type of the State
    enum stateType {
      /** \brief There are (so far) no outgoing Transitions.
       */
      NOTDEC,
      /** \brief States with at least one outgoing Transition, and all outgoing
       * Transitions are markov transitions.
       */
      MARKOV,
      /** \brief States with at least one outgoing Transition, and all outgoing
       * Transitions are interactive.
       */
      INTERACTIVE,
      /** \brief States with at least one outgoing Transition, and there are
       * both interactive and markov outgoing Transitions.
       */
      HYBRID
    };

    /// Standard constructor
    State();

    /// Destructor
    ~State();


    /** \brief Compute the markov successors of the State
     *
     * Calling this method only makes sense for an interactive State.
     * The computation of markov successors is by a DFS.
     * The Markov successors of a State are all markov States, that are
     * directly reached via a transition, and additionally the markov
     * successors of each interactive State, that is directly reached.
     *
     * @param computeLabels should the labels (i.e. the concatenated path to
     *                      the markov successor) be saved?
     */
    void getMarkovSuccs(const bool &computeLabels = true);
    
    /// Calculates new stateType based on the label of a new transition
    void determineStateType(const Transition *const &newTransition);

    /// Adds a new transition emanating this State
    void addTransition(Transition *const &newTransition);

    /** \brief Removes a transition
     *
     * The given iterator MUST point to a Transition emanating the State (i.e.
     * it is in the transitions-vector).
     * The return value is an iterator of the transition behind the deleted one
     * or behind the vector end, if the deleted was the last transition in the
     * vector.
     *
     * @param remove A non-constant iterator to the Transition to remove
     */
    std::vector<Transition*>::iterator removeTransition(std::vector<Transition*>::iterator &remove);
    
    /// Gives a pointer to the vector of emanating Transitions
    std::vector<Transition*>* getTransitions();
    /// Gives a pointer to the constant vector of emanating Transitions
    const std::vector<Transition*>* getTransitions() const;

    /// Returns the type of the State
    stateType getType() const;

    /** \brief Sets the type of the State
     *
     * Should be used with causion, because the IMC to CTMDP transformation
     * would most probably fail if the state type are wrong.
     *
     * @param newType The new type of the State
     */
    void setType(const stateType &newType);

    /** \brief Return the number of the State
     *
     * The numbering isn't done automatically, see Graph::numberStates.
     */
    unsigned int getNumber() const;
    /// Sets the number of the State
    void setNumber(const unsigned int &newNumber);

    /** \brief Set the interactive predecessor for the State
     *
     * In each State, the direct interactive predecessor is stored, for not
     * creating to many new States.
     *
     * @param newInteractivePred Pointer to an interactive predecessor of the
     *                           State
     */
    void setInteractivePred(State *const &newInteractivePred);

    /** \brief Returns the interactive predecessor of the State
     *
     * If no interactive predecessor is known (i.e. has not been set through
     * setInteractivePred()), a NULL pointer is returned.
     */
    State* getInteractivePred() const;

    /// Returns, if the State is marked
    bool getMark() const;

    /// Sets/cleans the mark of the State
    void setMark(const bool &newMark);

    /// Compare function, i.e. for sorting functions
    static bool compareByNr(State* firstState, State* secondState);

  private:

    bool markovSuccFinished; // did we do the 'DFS' for this state?

    // type of this State
    stateType type;

    // the output number of this State
    unsigned int number;

    // can this State do 'theAction'?
    bool mark;

    // INTERACTIVE predecessor of the state, NULL if none or unknown
    State* interactivePred;

    std::vector<Transition*> transitions; // store all transitions

};

#endif
