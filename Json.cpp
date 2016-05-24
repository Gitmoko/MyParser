/**
* @file
*
* @brief Demonstrates validation against a schema loaded from a file.
*
*/

#define PICOJSON_USE_INT64
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <picojson.h>

#include <valijson/utils/picojson_utils.hpp>
#include <valijson/adapters/picojson_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validation_results.hpp>
#include <valijson/validator.hpp>

using std::cerr;
using std::endl;

using valijson::Schema;
using valijson::SchemaParser;
using valijson::Validator;
using valijson::ValidationResults;
using valijson::adapters::PicoJsonAdapter;

int json(int argc, char *argv[])
{
	if (argc != 3) {
		cerr << "Usage: " << argv[0] << " <schema document> <test/target document>" << endl;
		return 1;
	}

	//Load the document containing the schema
	picojson::value schemaDocument;
	if (!valijson::utils::loadDocument(argv[1], schemaDocument)) {
		cerr << "Failed to load schema document." << endl;
		return 1;
	}


	// Load the document that is to be validated
	picojson::value targetDocument;
	if (!valijson::utils::loadDocument(argv[2], targetDocument)) {
		cerr << "Failed to load target document." << endl;
		return 1;
	}

	// Parse the json schema into an internal schema format
	Schema schema;
	SchemaParser parser;
	PicoJsonAdapter schemaDocumentAdapter(schemaDocument);
	try {
		parser.populateSchema(schemaDocumentAdapter, schema);
	}
	catch (std::exception &e) {
		cerr << "Failed to parse schema: " << e.what() << endl;
		return 1;
	}

	// Perform validation
	Validator validator(Validator::kWeakTypes);
	ValidationResults results;
	PicoJsonAdapter targetDocumentAdapter(targetDocument);
	if (!validator.validate(schema, targetDocumentAdapter, &results)) {
		std::cerr << "Validation failed." << endl;
		ValidationResults::Error error;
		unsigned int errorNum = 1;
		while (results.popError(error)) {

			std::string context;
			std::vector<std::string>::iterator itr = error.context.begin();
			for (; itr != error.context.end(); itr++) {
				context += *itr;
			}

			cerr << "Error #" << errorNum << std::endl
				<< "  context: " << context << endl
				<< "  desc:    " << error.description << endl;
			++errorNum;
		}
		return 1;
	}

	return 0;
}