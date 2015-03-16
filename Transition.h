#ifndef __TRANSITION_H
#define __TRANSITION_H

#include <string>
#include "Label.h"

// Forward-Declaration for class State
class State;

/** \brief Class for Transitions / Edges
 *
 * This Class encapsulates a labelled Transition and provides some methods to
 * retrieve basic properties.
 */
class Transition {
  public:
    /// Copy constructor
    Transition(const Transition&);
    /// Another constructor that sets target State and Label.
    Transition(State *const &target, const std::string &l);
    /// Another constructor that sets target State and Label.
    Transition(State *const &target, Label *const &l);

    /// Destructor
    ~Transition();

    /** \brief Decide if this transition is interactive or not.
     *
     * The decision is done from the Label.
     * See Label::isInteractive() for more detail.
     */
    bool isInteractive() const;

    /** \brief Decide if this transition is a \f$ \tau \f$ ("tau") transition.
     *
     * The decision is done from the Label.
     * See Label::isTau() for more detail.
     */
    bool isTau() const;

    /// Returns the target State of the Transition.
    State* const& getTargetState() const;

    /// Sets a new target State.
    void setTargetState(State *const &newTargetState);

    /// Return the Label-Object.
    Label* getLabel();
    /// Return the constant Label-Object.
    Label const* getLabel() const;

    /// Extract the rate of a markov Transition.
    double getRate() const;

    /// Sets a new Label.
    void setLabel(Label *const &newLabel);
    /// Sets a new Label.
    void setLabel(const std::string &newLabel);

    /// Equality operator
    bool operator==(const Transition &compTrans) const;

    /** \brief Compares two Transition pointers.
     *
     * Can be used i.e. as compare function for std::set<Transition*>
     */
    struct PtrComp {
      /// Compares the two Transition pointers by calling Transition::operator==.
      bool operator() (const Transition *const &t1, const Transition *const &t2);
    };

  private:
 
    State* targetState;

    Label* label;
};

#endif
