#include "Label.h"
#include "Graph.h"
#include <sstream>
#include <cassert>
#include <stdexcept>
#include <iomanip>

const std::string Label::separator = "|";

Label* Label::create(const std::string &text)
{
  // interactive label, if it consists of several parts
  bool severalParts = false;
  std::string::size_type posSeparator = 0;
  while ((posSeparator = text.find(separator, posSeparator)) != text.npos) {
    if ((posSeparator < 1) || (text[posSeparator-1] != '\\')) {
      severalParts = true;
      break;
    }
  }
  if (severalParts)
    return new LabelI(text);
  else if (text.substr(0, 5) == "rate ")
    return new LabelM(text.substr(5));
  else {
    std::string::size_type found = text.find("rate ", 1);
    if (found != std::string::npos)
      return new LabelM(text.substr(0, found), text.substr(found+5));
    else
      return new LabelI(text);
  }
}

Label::Label()
{
  // nothing to do
}

Label::~Label()
{
  // nothing to do
}

Label* Label::prepend(Label* label) {
  return prepend(label->str());
}

Label* Label::prepend(const std::string &newText) {
  return Graph::getLabelPtr(newText + separator + str());
}

LabelI::LabelI(const std::string &iText)
  : Label(), text(iText)
{
}

LabelI::~LabelI()
{
  text.clear();
}

bool LabelI::isInteractive() const
{
  return true;
}

bool LabelI::isTau() const
{
  return text == "i";
}

std::string LabelI::str()
{
  return text;
}

std::string Label::quote(const std::string &text)
{
  static std::string searchFor = separator+'\\';
  std::string::size_type loc = text.find_first_of(searchFor, 0);
  if (loc == std::string::npos)
    return text;
  else {
    std::string quoted(text, 0, loc);
    quoted.append("\\");
    std::string::size_type oldloc = loc;
    while ((loc = text.find_first_of(searchFor, oldloc+1)) != std::string::npos) {
      quoted.append(text, oldloc, loc-oldloc);
      quoted.append("\\");
      oldloc = loc;
    }
    if (oldloc < text.size())
      quoted.append(text, oldloc, text.size()-oldloc);
    return quoted;
  }
}

double LabelI::getRate() const
{
  assert(false);
  return 0.0;
}

LabelM::LabelM(const std::string &rateStr)
  : Label(), abstractName(NULL), fullText(NULL)
{
  std::istringstream t(rateStr);
  t >> rate;
  assert(t);
  std::string test;
  t >> test;
  if (t || !test.empty())
    throw std::runtime_error("there must be no text behind the rate");
}

LabelM::LabelM(const double &myRate)
  : Label(), rate(myRate), abstractName(NULL), fullText(NULL)
{
}

LabelM::LabelM(const std::string &abstrName,
    const std::string &rateStr)
  : Label(), abstractName(new std::string(abstrName)), fullText(NULL)
{
  std::istringstream t(rateStr);
  t >> rate;
  assert(t);
  if (!t) {
    std::ostringstream s;
    s << "Warning: invalid rate (non-numeric) \"" << rateStr << "\"";
    throw std::runtime_error(s.str());
  }
}

LabelM::~LabelM()
{
  if (abstractName)
    delete abstractName;
  if (fullText)
    delete fullText;
  abstractName = NULL;
  fullText = NULL;
}

bool LabelM::isInteractive() const
{
  return false;
}

bool LabelM::isTau() const
{
  return false;
}

std::string LabelM::str()
{
  if (!fullText) {
    std::ostringstream s;
    if (abstractName)
      s << *abstractName;
    s << "rate " << rate;
    fullText = new std::string(s.str());
  }
  return *fullText;
}

double LabelM::getRate() const
{
  return rate;
}
