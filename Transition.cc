#include "Transition.h"
#include "Graph.h"
#include <sstream>
#include <cassert>

Transition::Transition(const Transition &t)
  : targetState(t.targetState), label(t.label)
{
}

Transition::Transition(State *const &target, const std::string &l)
  : targetState(target)
{
  label = Graph::getLabelPtr(l);
}

Transition::Transition(State *const &target, Label *const &l)
  : targetState(target), label(l)
{
}

Transition::~Transition()
{
  // nothing to do, the label is destroyed by the Graph
}

bool Transition::isInteractive() const
{
  return label->isInteractive();
}

bool Transition::isTau() const
{
  return label->isTau();
}

State* const& Transition::getTargetState() const
{
  return targetState;
}

void Transition::setTargetState(State *const &newTargetState)
{
  targetState = newTargetState;
}

Label* Transition::getLabel()
{
  return label;
}

Label const* Transition::getLabel() const
{
  return label;
}

void Transition::setLabel(Label *const &newLabel)
{
  label = newLabel;
}

void Transition::setLabel(const std::string &newLabel)
{
  label = Graph::getLabelPtr(newLabel);
}

double Transition::getRate() const
{
  // can only be called for a markov Transition
  assert(!isInteractive());
  return label->getRate();
}

bool Transition::operator==(const Transition &compTrans) const
{
  return ((targetState == compTrans.targetState) && (label == compTrans.label));
}

bool Transition::PtrComp::operator() (const Transition *const &t1, const Transition *const &t2)
{
  return t1->targetState == t2->targetState
    ? t1->label < t2->label
    : t1->targetState < t2->targetState;
}

