#ifndef __LABEL_H
#define __LABEL_H

#include <string>

/** \brief Abstract base class for LabelI and LabelM
 *
 * Represents a label that can be associated with a Transition.
 */
class Label {
  public:

    /// Is this Label interactive?
    virtual bool isInteractive() const = 0;

    /** \brief Is this Label a \f$ \tau \f$ ("tau") label?
     *
     * By definition, a \f$ \tau \f$ Transition is a Transition with the
     * Label "i" (i.e. only the Label "i" returns true here).
     * (Only interactive Transitions may be \f$ \tau \f$ Transitions.)
     */
    virtual bool isTau() const = 0;

    /// Return a string representation of the Label.
    virtual std::string str() = 0;

    /** \brief Quote a label.
     *
     * In the returned string, each #separator and each '\' will
     * be pretended by a '\'.
     */
    static std::string quote(const std::string &text);

    /** \brief Returns the rate of the Label.
     * 
     * For interactive Labels, this method always returns 0.0.
     */
    virtual double getRate() const = 0;

    /** \brief Create a Label out of a string.
     *
     * This method is a factory for Label objects.
     * The returned object is either an instance of LabelI or LabelM,
     * depending of the string.
     * The Label will be a markov Label (LabelM) if it begins with
     * "rate " or it contains "rate " anywhere within the string. In the
     * latter case, the LabelM will contain an "abstract name", that will
     * be the part in front of the "rate ". See LabelM for further details.
     *
     * @param text The string to create the Label from
     */
    static Label* create(const std::string &text);
    
    /** \brief Prepend the Label with some other Label.
     * 
     * The same as prepend(label->str()). The Label instance will be
     * unchanged, a new Label is created and returned.
     */
    Label* prepend(Label* label);
    
    /** \brief Prepend the Label with some text.
     * 
     * The Label instance will be unchanged, a new (interactive) Label
     * is created and returned.
     * 
     * @param newText The text of the new Label will be this text +
     *                #separator + the text of the actual Label
     *                (return value of str()).
     */
    Label* prepend(const std::string &newText);

    /// Destruktor
    virtual ~Label();

  protected:

    /// Standardconstructor
    Label();

    /** \brief The separator of Label parts.
     * 
     * This constant is used e.g. in Label::prepend.
     */
    static const std::string separator;

};

/// An interactive Label.
class LabelI: public Label {

  public:

    /// Constructor
    LabelI(const std::string &text);

    /// Always true
    virtual bool isInteractive() const;

    /// True, if the text is exactly "i".
    virtual bool isTau() const;

    /// Returns the text of the LabelI
    virtual std::string str();

    /// Returns always 0.0
    virtual double getRate() const;
    
    /// Destructor
    virtual ~LabelI();
    
  private:

    std::string text;

};

/// A markov Label
class LabelM: public Label {

  public:

    /** \brief Constructor without an abstract name.
     * 
     * @param rateStr A string containing only the rate of the markov
     *                Label, e.g. "3.2" or "8.2e-3". There must be no text
     *                before or behind the rate.
     */
    LabelM(const std::string &rateStr);

    /** \brief Constructor with rate as double.
     * 
     * This is the preferred Constructor.
     * 
     * @param myRate The rate of the Transition associated with this Label.
     */
    LabelM(const double &myRate);

    /** \brief Constructor with an abstract name.
     * 
     * @param abstractName An abstract name for this markov Label.
     *                     See method str() for more detail.
     * @param rateStr See LabelM(const std::string&)
     */
    LabelM(const std::string &abstractName, const std::string &rateStr);

    /// Destruktor
    virtual ~LabelM();

    /// Always false
    virtual bool isInteractive() const;

    /// Always false
    virtual bool isTau() const;

    /** \brief Returns a string representation of this Label.
     *
     * The string will be the abstract name (is set), followed by "rate "
     * and the rate of the markov Label.
     */
    virtual std::string str();

    /// Returns the rate of the markov Label.
    virtual double getRate() const;

  private:

    double rate;

    std::string* abstractName;
    
    std::string* fullText;

};

#endif

