// ***********************************************************************
// Assembly         : HASHCOLON.Helper
// Author           : Wonchul Yoo
// Created          : 01-22-2018
//
// Last Modified By : Wonchul Yoo
// Last Modified On : 01-22-2018
// ***********************************************************************
// <copyright file="ProgramConst.hpp" company="">
//     Copyright (c) . All rights reserved.
// </copyright>
// <summary>Remodeled boost::property_tree for HASHCOLONLib. Used to read INI files.</summary>
// ***********************************************************************
#ifndef HASHCOLON_HELPER_PROGRAMCONST_HPP
#define HASHCOLON_HELPER_PROGRAMCONST_HPP

// to ensure thread safeness of ptree
#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>

#include <memory>
#include <mutex>
#include <map>
#include <string>
#include <queue>

#include <HashColon/Helper/ErrorCodes.hpp>

/// <summary>
/// The HASHCOLON namespace.
/// </summary>
namespace HASHCOLON
{
	/// <summary>
	/// The Helper namespace.
	/// </summary>
	namespace Helper
	{
		/// <summary>
		/// Class ProgramConst(Singleton).
		/// </summary>
		/// <seealso cref="boost::property_tree::ptree" />
		class ProgramConst : public boost::property_tree::ptree
		{
		private:
			static std::shared_ptr<ProgramConst> _instance;
			static std::once_flag _onlyOne;
			/// <summary>
			/// The property tree
			/// </summary>
			boost::property_tree::ptree _propertyTree;

			/// <summary>
			/// Initializes a new instance of the <see cref="ProgramConst"/> class.
			/// </summary>
			/// <param name="id">The identifier.</param>
			ProgramConst(int id) {
				//std::cout << "ProgramConstFactory::Singleton()" << id << std::endl;
			}

			/// <summary>
			/// Initializes a new instance of the <see cref="ProgramConst"/> class.
			/// </summary>
			/// <param name="rs">The rs.</param>
			ProgramConst(const ProgramConst& rs) {
				_instance = rs._instance;
			}

			/// <summary>
			/// Operator =.
			/// </summary>
			/// <param name="rs">The right hand side.</param>
			/// <returns>HASHCOLON.Helper.ProgramConst &.</returns>
			ProgramConst& operator = (const ProgramConst& rs)
			{
				if (this != &rs) {
					_instance = rs._instance;
				}

				return *this;
			}

		public:
			/// <summary>
			/// Finalizes an instance of the <see cref="ProgramConst"/> class.
			/// </summary>
			~ProgramConst() {
				//std::cout << "Singleton::~Singleton" << std::endl;
			}

			/// <summary>
			/// Gets the instance.
			/// </summary>
			/// <param name="id">The identifier.</param>
			/// <returns>HASHCOLON.Helper.ProgramConst &.</returns>
			static ProgramConst & GetInstance(int id = 0)
			{
				std::call_once(ProgramConst::_onlyOne,
					[](int idx)
				{
					ProgramConst::_instance.reset(new ProgramConst(idx));

					//std::cout << "ProgramConstFactory::create_singleton_() | thread id " + idx << std::endl;
				}, id);

				return *ProgramConst::_instance;
			}

			// copied from boost. 
			// As ini parser in boost failed to parse in-line comments,
			// I'm re-writing the function for myself.
/// <summary>
/// Reads the ini.
/// </summary>
/// <param name="stream">The stream.</param>
/// <param name="pt">The pt.</param>
			template<class Ptree>
			void read_ini(std::basic_istream<
				typename Ptree::key_type::value_type> &stream,
				Ptree &pt)
			{
				typedef typename Ptree::key_type::value_type Ch;
				typedef std::basic_string<Ch> Str;
				const Ch semicolon = stream.widen(';');
				const Ch hash = stream.widen('#');
				const Ch lbracket = stream.widen('[');
				const Ch rbracket = stream.widen(']');

				Ptree local;
				unsigned long line_no = 0;
				Ptree *section = 0;
				Str line;

				// For all lines
				while (stream.good())
				{

					// Get line from stream
					++line_no;
					std::getline(stream, line);
					if (!stream.good() && !stream.eof())
						BOOST_PROPERTY_TREE_THROW(
							boost::property_tree::ini_parser::ini_parser_error(
								"read error", "", line_no));

					// If line is non-empty
					line = boost::property_tree::detail::trim(line, stream.getloc());
					if (!line.empty())
					{
						// Comment, section or key?
						if (line[0] == semicolon || line[0] == hash)
						{
							// Ignore comments
						}
						else if (line[0] == lbracket)
						{
							// If the previous section was empty, drop it again.
							if (section && section->empty())
								local.pop_back();
							typename Str::size_type end = line.find(rbracket);
							if (end == Str::npos)
								BOOST_PROPERTY_TREE_THROW(
									boost::property_tree::ini_parser::ini_parser_error(
										"unmatched '['", "", line_no));
							Str key = boost::property_tree::detail::trim(
								line.substr(1, end - 1), stream.getloc());
							if (local.find(key) != local.not_found())
								BOOST_PROPERTY_TREE_THROW(
									boost::property_tree::ini_parser::ini_parser_error(
										"duplicate section name", "", line_no));
							section = &local.push_back(
								std::make_pair(key, Ptree()))->second;
						}
						else
						{
							Ptree &container = section ? *section : local;
							typename Str::size_type eqpos = line.find(Ch('='));
							typename Str::size_type commentpos, datalen;
							typename Str::size_type semicolonpos = line.find(semicolon);
							typename Str::size_type hashpos = line.find(hash);

							if (hashpos == Str::npos)
								commentpos = semicolonpos;
							else if (semicolonpos == Str::npos)
								commentpos = hashpos;
							else
								commentpos = std::min(semicolonpos, hashpos);

							if (commentpos == Str::npos)
								datalen = Str::npos;
							else if (commentpos > eqpos)
								datalen = commentpos - eqpos-1;
							else
								BOOST_PROPERTY_TREE_THROW(
									boost::property_tree::ini_parser::ini_parser_error(
										"key name cannot contain ; or #", "", line_no));



							if (eqpos == Str::npos)
								BOOST_PROPERTY_TREE_THROW(
									boost::property_tree::ini_parser::ini_parser_error(
										"'=' character not found in line", "", line_no));
							if (eqpos == 0)
								BOOST_PROPERTY_TREE_THROW(
									boost::property_tree::ini_parser::ini_parser_error(
										"key expected", "", line_no));


							Str key = boost::property_tree::detail::trim(
								line.substr(0, eqpos), stream.getloc());
							Str data = boost::property_tree::detail::trim(
								line.substr(eqpos + 1, datalen), stream.getloc());
							if (container.find(key) != container.not_found())
								BOOST_PROPERTY_TREE_THROW(
									boost::property_tree::ini_parser::ini_parser_error(
										"duplicate key name", "", line_no));
							container.push_back(std::make_pair(key, Ptree(data)));
						}
					}
				}
				// If the last section was empty, drop it again.
				if (section && section->empty())
					local.pop_back();

				// Swap local ptree with result ptree
				pt.swap(local);

			}

			// copied from boost. 
			// As ini parser in boost failed to parse in-line comments,
			// I'm re-writing the function for myself.
/// <summary>
/// Reads the ini.
/// </summary>
/// <param name="filename">The filename.</param>
/// <param name="pt">The pt.</param>
/// <param name="loc">The loc.</param>
			template<class Ptree>
			void read_ini(const std::string &filename,
				Ptree &pt,
				const std::locale &loc = std::locale())
			{
				std::basic_ifstream<typename Ptree::key_type::value_type>
					stream(filename.c_str());
				if (!stream)
					BOOST_PROPERTY_TREE_THROW(
						boost::property_tree::ini_parser::ini_parser_error(
							"cannot open file", filename, 0));
				stream.imbue(loc);
				try {
					read_ini(stream, pt);
				}
				catch (boost::property_tree::ini_parser::ini_parser_error &e) {
					BOOST_PROPERTY_TREE_THROW(
						boost::property_tree::ini_parser::ini_parser_error(
							e.message(), filename, e.line()));
				}
			}


			int AddOptionFile(std::string filepath)
			{
				// read new ptree
				std::shared_ptr<boost::property_tree::ptree> tmpPtree(new boost::property_tree::ptree());
				read_ini<boost::property_tree::ptree>(filepath, *tmpPtree);
				//	<boost::property_tree::ptree>(filepath, tmpPtree);

				// Keep track of keys and values (subtrees) in new property tree
				std::queue<std::string> qKeys;
				std::queue<boost::property_tree::ptree> qValues;
				qValues.push(*tmpPtree);

				// Iterate over second property tree
				while (!qValues.empty())
				{
					// Setup keys and corresponding values
					boost::property_tree::ptree ptree = qValues.front();
					qValues.pop();
					std::string keychain = "";
					if (!qKeys.empty())
					{
						keychain = qKeys.front();
						qKeys.pop();
					}

					// Iterate over keys level-wise
/// <summary>
/// Boosts the foreach.
/// </summary>
/// <param name="child">The child.</param>
/// <param name="">The .</param>
/// <returns>int.</returns>
					BOOST_FOREACH(const boost::property_tree::ptree::value_type& child, ptree)
					{
						// Leaf
						if (child.second.size() == 0)
						{
							// No "." for first level entries
							std::string s;
							if (keychain != "")
								s = keychain + "." + child.first.data();
							else
								s = child.first.data();

							// Put into combined property tree
							this->put(s, child.second.data());
						}
						// Subtree
						else
						{
							// Put keys (identifiers of subtrees) and all of its parents (where present)
							// aside for later iteration. Keys on first level have no parents
							if (keychain != "")
								qKeys.push(keychain + "." + child.first.data());
							else
								qKeys.push(child.first.data());

							// Put values (the subtrees) aside, too
							qValues.push(child.second);
						}
					}  // -- End of BOOST_FOREACH
				}  // --- End of while

				return _HASHCOLONErrCode_OK;
			}
		};

	}
}

// Macros
#define HASHCOLONCONST_ADDOPTIONFILE(FILEPATH) \
	HASHCOLON::Helper::ProgramConst::GetInstance().AddOptionFile(FILEPATH)
/*#define HASHCOLONCONST(TAG,TYPE,IDX) \ 
	HASHCOLON::Helper::ProgramConst::GetInstance(IDX).get<TYPE>(#TAG) */

#define HASHCOLONCONST(TAG,TYPE) \
	HASHCOLON::Helper::ProgramConst::GetInstance().get<TYPE>(#TAG)


#endif // !HASHCOLON_HELPER_PROGRAMOPTIONS_HPP

