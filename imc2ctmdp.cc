#include <cstdlib>
#include <string>
#include <getopt.h>
#include <sys/time.h>
#include <stdexcept>
#include <sstream>
#include <cassert>
#ifdef MALLINFO
  #include <malloc/malloc.h>
#endif

#include "imc2ctmdp.h"
#include "Graph.h"

void printHelp(const char* firstArg);
double timeDiff(const timeval&, const timeval&);
bool noColor(false);
char* colorGreen  = "\033[32m";
char* colorRed    = "\033[31m";
char* colorPurple = "\033[35m";
char* colorReset  = "\033[0m";
char* colorGrayBg = "\033[47m";

int main (int argc, char *argv[]) {

  timeval startTime;
#ifndef GETTIMEOFDAY_IS_ABSENT
  gettimeofday(&startTime, NULL);
#endif

  std::string inputFilename,
    outputFilenames,
    theAction;

  bool computeLabels(true), deleteUnreachable(false), cycleSearch(true),
       searchAbsorbing(false), uniformize(true);

  /*
   * Parsing the input arguments.
   */
  const struct option long_options[] = {
    { "action",             required_argument, 0, 'a' },
    { "no-color",           no_argument,       0, 'c' },
    { "delete-unreachable", no_argument,       0, 'd' },
    { "input",              required_argument, 0, 'i' },
    { "no-cycle-search",    no_argument,       0, 'k' },
    { "no-labels",          no_argument,       0, 'l' },
    { "output",             required_argument, 0, 'o' },
    { "no-uniformize",      no_argument,       0, 'n' },
    { "search-absorbing",   no_argument,       0, 's' },
    { "help"  ,             no_argument,       0, 'h' },
    { 0, 0, 0, 0 }
  };

  while (optind < argc) {
    int index = -1;
    int result = getopt_long(argc, argv, "a:cdi:klo:nsh",
                             long_options, &index);
    if (result == -1) break; // end of list
    switch (result) {
      case 'a':
        theAction = optarg;
        break;
      case 'c':
        noColor = true;
        break;
      case 'd':
        deleteUnreachable = true;
        break;
      case 'i':
        inputFilename = optarg;
        break;
      case 'k':
        cycleSearch = false;
        break;
      case 'l':
        computeLabels = false;
        break;
      case 'o':
        outputFilenames = optarg;
        break;
      case 'n':
        uniformize = false;
        break;
      case 's':
        searchAbsorbing = true;
        break;
      case 'h':
        printHelp(argv[0]);
        return 0;
      default:
        printf("Unknown parameter: \"%s\"\n", argv[optind]);
        break;
    }
  }
  while (optind < argc) {
    if (inputFilename.empty())
      inputFilename.assign(argv[optind++]);
    else
      printf("Unknown parameter: \"%s\"\n", argv[optind++]);
  }

  // exit, if no filename given
  if (inputFilename.empty()) {
    fprintf(stderr, "No filename given. Try \"-h\" for help.\n");
    exit(-1);
  }

  std::string default_filename(inputFilename);
  if (default_filename.rfind('.') != default_filename.npos)
    default_filename.erase(default_filename.rfind('.'));
  if (default_filename.find(':') != default_filename.npos)
    default_filename.erase(0, default_filename.find(':') + 1);
  default_filename.append("_ctmdp");

  if (outputFilenames.empty())
    outputFilenames = default_filename + ".bcg";

#ifndef NO_EXCEPTION_CATCH
  try {
#endif
    // set options
    Graph::setCycleSearch(cycleSearch);
    Graph::setSearchForAbsorbingStates(searchAbsorbing);
    Graph::setAction(theAction);
    if (noColor) {
      colorGreen = colorRed = colorPurple = colorReset = colorGrayBg =
        new char[1];
      colorGreen[0] = '\0';
      Graph::setColorDebug("\033[0m");
      Graph::setColorWarning("\033[0m");
    }

    countTime("Reading input file");
    std::pair<std::string, std::string> inputFormat = getFormat(inputFilename,
        default_filename);
    if (inputFormat.first.empty()) {
      //Graph::warn("Warning: Couldn't extract format of file \"%s\".",
      //    inputFilename.c_str());
      //exit(-1);
      inputFilename.append(".bcg");
      inputFormat.first = "bcg";
      inputFormat.second = inputFilename;
    }
    Graph* graph = Graph::read(inputFormat.first, inputFormat.second);
    if (!graph) {
      graph->warn("Error reading input file, exiting.");
      exit(-1);
    }

    if (deleteUnreachable) {
      countTime("Deleting unreachable states (#1, before transformation)");
      graph->deleteUnreachable();
    }

    countTime("Checking uniformity");
    {
      double maxOutgoingRate;
      bool uniform = graph->checkUniformity(true, &maxOutgoingRate);
      if (!uniform && uniformize) {
        countTime("Uniformizing IMC");
        graph->uniformize(maxOutgoingRate);
        assert(graph->checkUniformity(true));
      }
    }

    countTime("Transforming IMC to CTMDP");
    graph->transformImcToCtmdp(computeLabels);

    if (deleteUnreachable) {
      countTime("Deleting unreachable states (#2, after transformation)");
      graph->deleteUnreachable();
    }

    countTime("Checking for internal nondeterminism");
    if (graph->hasInternalNondeterminism())
      graph->warn("Warning: Graph has internal nondeterminism");

    countTime("Preparing Graph for output");
    // get the Graph ready for export
    graph->prepareForExport();
    graph->setReadyForExport(true);

    while (!outputFilenames.empty()) {
      std::string filename;
      std::string::size_type comma = outputFilenames.find(',');
      if (comma != outputFilenames.npos) {
        filename = outputFilenames.substr(0, comma);
        outputFilenames.erase(0, comma + 1);
      } else {
        filename = outputFilenames;
        outputFilenames.erase();
      }
      std::pair<std::string, std::string> outputFormat = getFormat(filename,
          default_filename);
      if (outputFormat.first.empty()) {
        outputFormat.first = "bcg";
        outputFormat.second.append(".bcg");
      }
      std::ostringstream debug;
      debug << "Writing " << outputFormat.first << " output to \""
        << outputFormat.second << "\"";
      countTime(debug.str().c_str());
      graph->write(outputFormat.first, outputFormat.second);
    }

    countTime("Ready");

    // delete the Graph
    delete graph;

    // print some Timing information
    timeval endTime;
#ifndef GETTIMEOFDAY_IS_ABSENT
    gettimeofday(&endTime, NULL);
#endif
    clock_t cpuClock = clock();
    printf("%sTransformation took %.2f seconds of CPU time, %.2f real time.\n%s",
        colorGreen, 1.0 * cpuClock / CLOCKS_PER_SEC,
        timeDiff(startTime, endTime), colorReset);

#ifdef MALLINFO
    struct mallinfo memInfo = mallinfo();
    unsigned long memUsed = memInfo.arena; // uordblks + memInfo.fordblks;
    printf("%sMemory in usage: %.2f MB\n%s",
        colorGreen, 1e-6*memUsed, colorReset);
#endif

    return 0;

#ifndef NO_EXCEPTION_CATCH
  }
  catch(std::bad_alloc&) {
    printf("%s%sNo more memory available.\n"
        "Try option '-l' (see '-h' for help).%s\n",
        colorRed, colorGrayBg, colorReset);
  }
  catch (std::exception &e) {
    printf("%s%sAn exception occured: %s%s\n",
        colorRed, colorGrayBg, e.what(), colorReset);
  }
  catch (...) {
    printf("%s%sAn UNKNOWN EXCEPTION occured!!%s\n",
        colorRed, colorGrayBg, colorReset);
  }
#endif

  return -1;
}

void countTime(const char* text)
{
#ifndef NOTIMING
  static timeval lastTime, firstTime;
  static bool first = true;
  if (first) {
    first = false;
#ifndef GETTIMEOFDAY_IS_ABSENT
    gettimeofday(&lastTime, NULL);
    gettimeofday(&firstTime, NULL);
#endif
  }

  timeval curTime;
#ifndef GETTIMEOFDAY_IS_ABSENT
  gettimeofday(&curTime, NULL);
#endif
  printf("%s+ %s (%.2f sec, +%.2f sec)%s\n", colorPurple, text, 1.0f* timeDiff(
      firstTime, curTime), 1.0f* timeDiff(lastTime, curTime), colorReset);
  fflush(stdin);

  lastTime = curTime;
#endif
}

double timeDiff(const timeval &t1, const timeval &t2) {
  return t2.tv_sec - t1.tv_sec+ 1e-6*(t2.tv_usec - t1.tv_usec);
}

std::pair<std::string, std::string> getFormat(const std::string &filename,
    const std::string &default_filename)
{
  std::string::size_type pos = filename.find(':');
  if (pos == filename.size() - 1)
    return std::make_pair(filename.substr(0, pos),
        default_filename + "." + filename.substr(0, pos));
  if (pos != filename.npos)
    return std::make_pair(filename.substr(0, pos), filename.substr(pos+1));
  pos = filename.rfind('.');
  if (pos != filename.npos)
    return std::make_pair(filename.substr(pos+1), filename);

  return std::make_pair(std::string(), filename);
}

void printHelp(const char* firstArg)
{
  const char *help = "Usage: %s <parameters> [inputFilename[.bcg]]\n"
"where <parameters> can be:\n"
"\n"
"   -a, --action=STRING\n"
"      sets the label that markes the interesting states\n"
"\n"
"   -c, --no-color\n"
"      do not print colored debug messages and warnings\n"
"\n"
"   -d, --delete-unreachable\n"
"      search for and delete unreachable states\n"
"\n"
"   -i, --input=filename[.bcg]\n"
"      the input filename. If no extension is given, \".bcg\" will be appended"
"\n"
"\n"
"   -k, --no-cycle-search\n"
"      don't search for interactive cycles (do only use if you know that no\n"
"      interactive cycles exist, otherwise the program may fail)\n"
"\n"
"   -l, --no-labels\n"
"      don't compute labels for markov successors of interactive states\n"
"\n"
"   -n, --no-uniformize\n"
"      per default, the IMC is uniformized. By this option, you can\n"
"      disable this feature\n"
"\n"
"   -o, --output=filenames\n"
"      see below for more information\n"
"\n"
"   -s, --search-absorbing\n"
"      search for absorbing states and label them in the .lab file\n"
"\n"
"   -h, --help\n"
"      print this help\n"
"\n"
"This program reads the IMC (interactive Markov chain) from the given\n"
"input filename, converts it to a CTMDP (continuous-time Markov decision\n"
"process) and writes it to the given output filenames.\n"
"If no output filenames are specified, the output is written as BCG to the\n"
"input filename without extension and extended by \"_ctmdp.bcg\".\n"
"The given list of output filenames is a comma separated list of (possibly)\n"
"different output formats and filenames.\n"
"Each entry has the form   <format>:<filename>.<extension>\n"
"If the format (and the colon) is omitted, it is extracted from the extension.\n"
"If additionally the extension is omitted, it is assumed to be \".bcg\".\n"
"If the filename and the extension are omitted, the filename will be the same\n"
"as the input filename, but extended by \"_ctmdp\" and as extension the\n"
"format string.\n"
;

  printf(help, firstArg);

  printf("\nAvailable input formats: ");
  std::vector<std::string> inputFormats = Graph::getInputFormats();
  for (std::vector<std::string>::const_iterator it = inputFormats.begin();
      it != inputFormats.end(); ++it)
    printf("%s%s", it == inputFormats.begin() ? "" : ", ", it->c_str());

  printf("\nAvailable output formats: ");
  std::vector<std::string> outputFormats = Graph::getOutputFormats();
  for (std::vector<std::string>::const_iterator it = outputFormats.begin();
      it != outputFormats.end(); ++it)
    printf("%s%s", it == outputFormats.begin() ? "" : ", ", it->c_str());

  printf("\n\n");

}




// Mainpage Documentation for doxygen:

/** \mainpage Description of the transformation
 *
 * \section mainpage_imc2ctmdp IMC2CTMDP
 *
 * This program converts an IMC into an "equivalent" CTMDP. The transformation
 * steps are described in the document \b ctmdp.pdf.
 * The main work is done in the method Graph::transformImcToCtmdp.
 *
 * For possible input formats see the derived classes of GraphInput,
 * for output formats those of GraphOutput.
 *
 * \sa ctmdp.pdf
 * \sa http://www.inrialpes.fr/vasy/pub/cadp/man/bcg.html
 */
