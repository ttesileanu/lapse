/** @file lapse.cc
 *  @brief A small program to help with the creation of smooth timelapses.
 *
 *  The program can handle keypoint-based transitions in various quantities,
 *  such as exposure.
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include "misc/fixwidthmanip.h"
#include "processor.h"

const char version_string[] = "0.1";

namespace po = boost::program_options;

namespace {  // anonymous namespace

void setup_options(po::options_description& options)
{
  options.add_options()
    ("help,h", "show this message")
    ("verbosity,v", po::value<int>() -> default_value(1),
      "select verbosity level")
    ("quiet,q", "set verbosity to 0")
    ("version", "show program version")
    ("single,s", "single file processing")
    ("file",  po::value<strings>(), "alias for positional arguments")
    ("effects,e", po::value<std::string>(),
      "list of keyframed effects to be executed")
    ("effects-file,f", po::value<std::string>(),
      "get list of effects from file")
    ("output,o", po::value<std::string>(),
      "format for output files, in the form [path/]nameXXXX.ext; the X's will "
      "be replaced with numbers from 0 to the total number of frames minus 1.");
}

struct usage_information {
  usage_information(const std::string& name) : program_name(name) {}

  std::string program_name;
};

std::ostream& operator<<(std::ostream& out, const usage_information& i)
{
  out << "Usage:" << std::endl;
  out << i.program_name << " [options] <first_file> <last_file> [<first_file> "
      << "<last_file> ...]";
  out << std::endl << "  or" << std::endl;
  out << i.program_name << " -s [options] <single_file>" << std::endl;
  out << std::endl;
  out << fixwidth() << "In the first form, this program processes Jpeg files "
      << "for creating timelapses. The files should have names of the form "
      << "<prefix>XX..XX.<extension>, where X is a digit. The number of digits "
      << "contained in <first_file> and <last_file> should match; all of the "
      << "files in-between will be considered for processing. If several pairs "
      << "<first_file>, <last_file> are given, all the corresponding ranges "
      << "are concatenated." << std::endl << std::endl;
  out << fixwidth() << "In the second form, a single file is processed. This "
      << "can be used to test the timelapse parameters." << std::endl;

  return out;
}

struct SplitStructure {
  std::string   prefix;
  std::string   suffix;
  int           ndigits;
  int           n;
};

SplitStructure split_file(const std::string& name)
{
  if (name.empty())
    throw std::runtime_error("Empty file name.");

  SplitStructure res;

  // find the extension, if any
  // XXX this isn't great: if name is a path, the dot could belong to one of
  // the directories in the path, as opposed to the actual file name
  const size_t dpos = name.rfind('.');
  std::string presuffix;
  if (dpos != std::string::npos && dpos != 0) {
    // dpos != 0: if the file name starts with a dot (and that's the only dot),
    // don't treat it as an extension
    res.suffix = name.substr(dpos);
    presuffix = name.substr(0, dpos);
  } else {
    presuffix = name;
  }
  
  // find the number and the prefix
  size_t nstart = presuffix.find_last_not_of("0123456789");
  if (nstart == std::string::npos)
    nstart = 0;
  else
    ++nstart;
  res.prefix = presuffix.substr(0, nstart);

  res.ndigits = presuffix.length() - nstart;
  if (res.ndigits > 0)
    res.n = boost::lexical_cast<int>(presuffix.substr(nstart));
  else
    res.n = 0;

  return res;
}

} // anonymous namespace

int main(int argc, const char* argv[])
{
#ifdef __APPLE__
  // this is for debugging, because on MacOS X, exception text is not displayed
  try {
#endif
  po::options_description options("Command-line options");
  setup_options(options);

  po::variables_map params;
  po::store(po::command_line_parser(argc, argv).options(options).
    positional(po::positional_options_description().add("file", -1)).run(),
    params);
  po::notify(params);

  if (params.count("help")) {
    std::cout << usage_information(argv[0]) << std::endl;
    std::cout << options << std::endl;
    return 0;
  }
  if (params.count("version")) {
    std::cout << version_string << std::endl;
    return 0;
  }

  // The input needs to specify keframes and various settings that will
  // interpolate between them. Commands in-between keyframes should control
  // the interpolation method. Represent a keyframe by a number (base 0)
  // followed by a colon. The next commands all set the settings for that
  // keyframe. The commands use a hierarchical structure separated by dots, so
  // that the properties associated to a common transformation are grouped
  // together. Interpolation properties are defined the same way; they affect
  // the interpolation between the keyframe where they are inserted and
  // subsequent frames. Interpolation for a given transformation happens only
  // between keyframes that set properties for that transformation.
  
  // Effects are applied in the order in which they appear the very first time.
  // Values of properties that are not altered by a keyframe keep their values
  // from the previous keyframe. Before a property is first mentioned, its
  // value is kept at its default -- so you need to specify the property in
  // at least two keyframes to get a smooth interpolation.

  if (!params.count("file")) {
    std::cerr << "Need some input files." << std::endl << std::endl;
    std::cerr << usage_information(argv[0]) << std::endl;
    std::cerr << options << std::endl;
    return 1;
  }
  if (params.count("single") && params["file"].as<strings>().size() != 1) {
    std::cerr << fixwidth() << "In single operation mode, a single input file "
              << "is expected. Try " << argv[0] << " --help for details."
              << std::endl;
    return 1;
  }
  if (!params.count("single") && params["file"].as<strings>().size() % 2 != 0) {
    std::cerr << fixwidth() << "Files should come in pairs of first_file, "
              << "last_file. Try " << argv[0] << " --help for details."
              << std::endl;
    return 1;
  }
  if (!params.count("output")) {
    std::cerr << "Need an output file name template." << std::endl;
    std::cerr << usage_information(argv[0]) << std::endl;
    std::cerr << options << std::endl;
    return 1;
  }

  // read in the list of files
  strings file_names;
  if (params.count("single")) {
    file_names.push_back(params["file"].as<strings>().front());
  } else {
    const strings& file_args = params["file"].as<strings>();
    const size_t nseqs = file_args.size() / 2;
    for (size_t i = 0; i < nseqs; ++i) {
      const std::string& first_file = file_args[2*i];
      const std::string& last_file = file_args[2*i + 1];

      SplitStructure first_struct = split_file(first_file);
      SplitStructure last_struct = split_file(last_file);
      if (first_struct.prefix != last_struct.prefix ||
          first_struct.suffix != last_struct.suffix ||
          first_struct.ndigits != last_struct.ndigits) {
        std::cerr << fixwidth() << "Non-matching pair of file names ("
                  << first_file << ", " << last_file << ")." << std::endl;
        return 1;
      }

      if (first_struct.n > last_struct.n) {
        std::cerr << fixwidth() << "File numbers need to be increasing ("
                  << first_file << ", " << last_file << ")." << std::endl;
        return 1;
      }

      for (size_t n = first_struct.n; n <= last_struct.n; ++n) {
        std::ostringstream sstream;
        if (first_struct.ndigits > 0)
          sstream << std::setfill('0') << std::setw(first_struct.ndigits) << n;
        file_names.push_back(first_struct.prefix + sstream.str()
          + first_struct.suffix);
      }
    }
  }

  // check that the files exist before we start doing any work
  strings missing;
  const size_t max_missing = 5;
  for (std::string s: file_names) {
    std::ifstream f(s.c_str());
    if (!f.good()) {
      missing.push_back(s);
      if (missing.size() > max_missing) break;
    }
  }
  if (!missing.empty()) {
    std::ostringstream sstream;
    sstream << "Some files are missing or unreadable (";
    for (size_t i = 0; i < std::min(max_missing, missing.size()); ++i) {
      if (i > 0) sstream << ", ";
      sstream << missing[i];
    }
    if (missing.size() > max_missing) sstream << ", ...";
    sstream << ").";
    std::cerr << fixwidth() << sstream.str() << std::endl;
    return 1;
  }

  // check how we're given the keyframed effects information
  std::string effects_str;
  if (params.count("effects")) {
    if (params.count("effects-file")) {
      std::cerr << "Please specify either --effects or --effects-file, not "
                << "both." << std::endl;
      return 1;
    }
    effects_str = params["effects"].as<std::string>();
  } else if (params.count("effects-file")) {
    std::ifstream f(params["effects-file"].as<std::string>().c_str());
    std::stringstream buffer;
    buffer << f.rdbuf();
    effects_str = buffer.str();
  }

  // start processing!
  Processor processor;
  processor.set_verbosity(params.count("quiet")?0:
    params["verbosity"].as<int>());
  processor.set_output(params["output"].as<std::string>());
  processor.add_files(file_names);
  processor.parse_effects(effects_str);

  processor.run();

#ifdef __APPLE__
  }
  catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
  }
  catch (...) {
    std::cout << "Exception not inheriting from std::exception thrown."
              << std::endl;
  }
#endif

  return 0;
}
