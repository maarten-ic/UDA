#include "nameValueSubstitution.h"

#include <cerrno>
#include <cstdlib>

#if defined(__GNUC__)
#  include <strings.h>
#  include <unistd.h>
#endif

#include "clientserver/errorLog.h"
#include "clientserver/makeRequestBlock.h"
#include "clientserver/stringUtils.h"
#include "initStructs.h"
#include "logging/logging.h"
#include "udaErrors.h"
#include <fmt/format.h>

using namespace uda::client_server;
using namespace uda::logging;

static void embedded_value_substitution(NameValueList* nameValueList);

// Deconstruct the text pass parameter (tpass) for name value placeholder substitution values
// Identify name value placeholders in the signal argument and replace with substitution values
// Add additional name-value pairs and keywords to the plugin input

// shot/tpass data source pattern: "12345/a,b,c, name=value, name=value, d, e, delimiter=',', placeholder='$'"
//

int uda::client_server::name_value_substitution(NameValueList* nameValueList, char* tpass)
{
    int err = 0;

    if (!tpass && strlen(tpass) == 0) {
        return 0; // nothing to substitute and no new NV pairs
    }

    UDA_LOG(UDA_LOG_DEBUG, "nameValueSubstitution - Initial set of %d NV pairs\n", nameValueList->pairCount);
    for (int i = 0; i < nameValueList->pairCount; i++) {
        UDA_LOG(UDA_LOG_DEBUG, "Pair[%d] = %s, Name = %s, value = %s\n", i, nameValueList->nameValue[i].pair,
                nameValueList->nameValue[i].name, nameValueList->nameValue[i].value);
    }

    NameValueList newNameValueList;
    init_name_value_list(&newNameValueList);

    unsigned short strip = 0; // Do Not Remove enclosing quotes from name value pairs
    if (name_value_pairs(tpass, &newNameValueList, strip) == -1) {
        err = 999;
        add_error(UDA_CODE_ERROR_TYPE, "nameValueSubstitution", err, "Name Value pair syntax is incorrect!");
        return err;
    }

    if (newNameValueList.pairCount == 0) { // No passed substitution values or additional name value pairs
        if (newNameValueList.nameValue != nullptr) {
            free_name_value_list(&newNameValueList);
        }
        return 0;
    }

    UDA_LOG(UDA_LOG_DEBUG, "nameValueSubstitution - Additional set of %d NV pairs\n", newNameValueList.pairCount);
    for (int i = 0; i < newNameValueList.pairCount; i++) {
        UDA_LOG(UDA_LOG_DEBUG, "Pair[%d] = %s, Name = %s, value = %s\n", i, newNameValueList.nameValue[i].pair,
                newNameValueList.nameValue[i].name, newNameValueList.nameValue[i].value);
    }

    // How many placeholders? [beginning with a '$' character. Placeholders may be numbered, named or simply a single
    // naked $ character]

    int placeholderCount = 0;
    int* placeholderIndex = nullptr;
    int* tpassIndex = nullptr;
    int tpassPosition = 0;

    if (nameValueList->pairCount > 0) { // Identify and Count Placeholders
        do {
            placeholderIndex = (int*)malloc(nameValueList->pairCount * sizeof(int));
            tpassIndex = (int*)malloc(nameValueList->pairCount * sizeof(int));

            for (int i = 0; i < nameValueList->pairCount; i++) {
                // Is it a placeholder? (Not a keyword and begining '$')
                if (nameValueList->nameValue[i].value != nullptr && nameValueList->nameValue[i].value[0] == '$') {
                    placeholderIndex[placeholderCount] = i; // Identify which pair
                    tpassIndex[placeholderCount] =
                        tpassPosition++; // Ordering: Default substitution value to use - list position
                    if (nameValueList->nameValue[i].value[1] != '\0') {
                        if (is_number(&nameValueList->nameValue[i]
                                          .value[1])) { // Is the placeholder numbered? - Use the specific substitution
                                                        // value identified by list order
                            tpassIndex[placeholderCount] = atoi(&nameValueList->nameValue[i].value[1]) -
                                                           1; // Position labelling begins '1' not '0'
                        } else {
                            // scan the list of substitute pairs and match by name (case sensitive)
                            for (int j = 0; j < newNameValueList.pairCount; j++) {
                                if (strcmp(&nameValueList->nameValue[i].value[1], newNameValueList.nameValue[j].name) ==
                                    0) {
                                    tpassIndex[placeholderCount] = j;
                                    break;
                                }
                            }
                        }
                    }
                    placeholderCount++;
                    if (placeholderCount <= newNameValueList.pairCount) {
                        UDA_LOG(UDA_LOG_DEBUG, "[%d] name: %s, substitution position %d, substitution value %s\n",
                                placeholderCount, nameValueList->nameValue[i].name, tpassIndex[placeholderCount - 1],
                                newNameValueList.nameValue[tpassIndex[placeholderCount - 1]].value);
                    }
                }
            }

            if (placeholderCount == 0) {
                break; // There are no placeholders to process
            }

            UDA_LOG(UDA_LOG_DEBUG, "Name Value Pair Count = %d\n", nameValueList->pairCount);
            UDA_LOG(UDA_LOG_DEBUG, "Placeholder Count     = %d\n", placeholderCount);
            UDA_LOG(UDA_LOG_DEBUG, "Additional Name Value Pairs and Substitutions = %s\n", tpass);
            UDA_LOG(UDA_LOG_DEBUG, "Additional Name Value Pairs and Substitutions Count = %d\n",
                    newNameValueList.pairCount);

            // Placeholder substitutions (must be the specified first in tpass)

            if (placeholderCount > newNameValueList.pairCount) {
                // Too many placeholders for the available substitutions
                UDA_LOG(UDA_LOG_DEBUG, "Inconsistent count of placeholders and available substitutions!\n");
                err = 999;
                add_error(UDA_CODE_ERROR_TYPE, "nameValueSubstitution", err,
                          "Inconsistent count of placeholders and available substitutions!");
                break;
            }

            // Replace placeholders with identifed values
            // Replacement values can be used multiple times

            for (int i = 0; i < placeholderCount; i++) {
                if (tpassIndex[i] < 0 || tpassIndex[i] > newNameValueList.pairCount) {
                    UDA_LOG(UDA_LOG_DEBUG, "Placeholder numbering is Inconsistent with Placeholder Count!\n");
                    UDA_LOG(UDA_LOG_DEBUG, "tpassIndex[%d] = %d  (%d)\n", i, tpassIndex[i], placeholderCount);
                    err = 999;
                    add_error(UDA_CODE_ERROR_TYPE, "nameValueSubstitution", err,
                              "Placeholder numbering is Inconsistent with Placeholder Count!");
                    break;
                }

                if (nameValueList->nameValue[placeholderIndex[i]].value != nullptr) {
                    free(nameValueList->nameValue[placeholderIndex[i]].value);
                }
                nameValueList->nameValue[placeholderIndex[i]].value = newNameValueList.nameValue[tpassIndex[i]].value;
                newNameValueList.nameValue[tpassIndex[i]].value = nullptr;

                UDA_LOG(UDA_LOG_DEBUG, "Placeholder: [%d][%d] %s, Substitution Value [%d] %s\n", i, placeholderIndex[i],
                        nameValueList->nameValue[placeholderIndex[i]].name, tpassIndex[i],
                        nameValueList->nameValue[placeholderIndex[i]].value);
            }

            // Remove all used NV pairs used in placeholder substitution
            for (int i = 0; i < placeholderCount; i++) {
                if (newNameValueList.nameValue[tpassIndex[i]].pair != nullptr) {
                    free(newNameValueList.nameValue[tpassIndex[i]].pair);
                }
                if (newNameValueList.nameValue[tpassIndex[i]].name != nullptr) {
                    free(newNameValueList.nameValue[tpassIndex[i]].name);
                }
                if (newNameValueList.nameValue[tpassIndex[i]].value != nullptr) {
                    free(newNameValueList.nameValue[tpassIndex[i]].value);
                }
                newNameValueList.nameValue[tpassIndex[i]].pair = nullptr;
                newNameValueList.nameValue[tpassIndex[i]].name = nullptr;
                newNameValueList.nameValue[tpassIndex[i]].value = nullptr;
            }

        } while (0);
    }

    free(tpassIndex);
    free(placeholderIndex);
    if (err != 0) {
        free_name_value_list(&newNameValueList);
        return err;
    }

    // Add unused new name value pairs and keywords

    if (nameValueList->pairCount + newNameValueList.pairCount > nameValueList->listSize) {
        nameValueList->nameValue =
            (NameValue*)realloc((void*)nameValueList->nameValue,
                                (nameValueList->listSize + newNameValueList.pairCount) * sizeof(NameValue));
        nameValueList->listSize = nameValueList->listSize + newNameValueList.pairCount;
    }

    for (int i = 0; i < newNameValueList.pairCount; i++) {
        if (newNameValueList.nameValue[i].name != nullptr) {
            nameValueList->nameValue[nameValueList->pairCount].pair = newNameValueList.nameValue[i].pair;
            if (nameValueList->nameValue[nameValueList->pairCount].pair == nullptr) {
                nameValueList->nameValue[nameValueList->pairCount].pair = strdup("");
            }
            nameValueList->nameValue[nameValueList->pairCount].name = newNameValueList.nameValue[i].name;
            nameValueList->nameValue[nameValueList->pairCount++].value = newNameValueList.nameValue[i].value;
            UDA_LOG(UDA_LOG_DEBUG, "[%d] Name = %s, Value = %s\n", i, newNameValueList.nameValue[i].name,
                    newNameValueList.nameValue[i].value);
            newNameValueList.nameValue[i].pair = nullptr;
            newNameValueList.nameValue[i].name = nullptr;
            newNameValueList.nameValue[i].value = nullptr;
        }
    }

    free_name_value_list(&newNameValueList);

    // Scan all values for embedded placeholders and substitute

    embedded_value_substitution(nameValueList);

    return 0;
}

// Substitute named placeholders within value strings

// patterns with name values: string="UDA::getdata(variable='/a/b/c', shot=$shot, tstart=$tstart, tend=$tend)"
// substitution for named string elements: $shot, $tstart, $tend

void embedded_value_substitution(NameValueList* nameValueList)
{
    int m;
    NameValueList newNameValueList;

    if (nameValueList->pairCount == 0) {
        return;
    }

    UDA_LOG(UDA_LOG_DEBUG, "embeddedValueSubstitution\n");
    for (int i = 0; i < nameValueList->pairCount; i++) {
        UDA_LOG(UDA_LOG_DEBUG, "Pair[%d] = %s, Name = %s, value = %s\n", i, nameValueList->nameValue[i].pair,
                nameValueList->nameValue[i].name, nameValueList->nameValue[i].value);
    }

    // Search all NV pairs for values with placeholders

    for (int i = 0; i < nameValueList->pairCount; i++) {

        char* p = strchr(nameValueList->nameValue[i].value, '$'); // Is there a placeholder?
        if (!p) {
            continue;
        }

        init_name_value_list(&newNameValueList);

        UDA_LOG(UDA_LOG_DEBUG, "Extracting Name Value Pairs from [%d]: %s\n", i, nameValueList->nameValue[i].value);

        unsigned short strip = 1; // Remove enclosing quotes from name value pairs
        char* work = strdup(nameValueList->nameValue[i].value);
        UDA_LOG(UDA_LOG_DEBUG, "[%d] work = %s\n", i, work);

        // Extract NV pairs and keywords from the input placeholder value

        int rc = name_value_pairs(work, &newNameValueList, strip);
        free(work);
        if (rc == -1) {
            continue;
        }

        UDA_LOG(UDA_LOG_DEBUG, "Embedded Pair Count = %d\n", newNameValueList.pairCount);
        for (int j = 0; j < newNameValueList.pairCount; j++) {
            UDA_LOG(UDA_LOG_DEBUG, "Pair[%d] %s, Name = %s, value = %s\n", j, newNameValueList.nameValue[j].pair,
                    newNameValueList.nameValue[j].name, newNameValueList.nameValue[j].value);
        }

        for (int j = 0; j < newNameValueList.pairCount; j++) {

            if (newNameValueList.nameValue[j].value[0] == '$') {
                // Match with substitution value
                for (int k = 0; k < nameValueList->pairCount; k++) {
                    if (k == i) {
                        continue;
                    }

                    // Match by name (case sensitive) only
                    if (!strcmp(&newNameValueList.nameValue[j].value[1], nameValueList->nameValue[k].name)) {

                        UDA_LOG(UDA_LOG_DEBUG, "Substitution: embedded[%d] %s=%s with [%d] %s=%s\n", j,
                                newNameValueList.nameValue[j].name, newNameValueList.nameValue[j].value, k,
                                nameValueList->nameValue[k].name, nameValueList->nameValue[k].value);

                        // Substitute into original string and replace original

                        // Substitute here with nameValueList->nameValue[k].value
                        char* original = strdup(nameValueList->nameValue[i].value);
                        UDA_LOG(UDA_LOG_DEBUG, "Original: %s\n", original);

                        char* pp = strstr(original, newNameValueList.nameValue[j].value);
                        int lstr = strlen(newNameValueList.nameValue[j].value); // Target this
                        int ok = 1;

                        UDA_LOG(UDA_LOG_DEBUG, "targeting %s [%d] from %s to %s\n", pp, lstr,
                                newNameValueList.nameValue[j].value, nameValueList->nameValue[k].value);

                        for (m = 0; m < lstr; m++) {
                            ok = ok && pp[m] == newNameValueList.nameValue[j].value[m]; // Test the name is correct
                        }

                        if (ok) {
                            UDA_LOG(UDA_LOG_DEBUG, "Substituting %s with %s\n", newNameValueList.nameValue[j].value,
                                    nameValueList->nameValue[k].value);

                            pp[0] = '\0';
                            std::string replace =
                                fmt::format("{}{}{}", original, nameValueList->nameValue[k].value, &pp[lstr]);

                            UDA_LOG(UDA_LOG_DEBUG, "original %s\n", original);
                            UDA_LOG(UDA_LOG_DEBUG, "value    %s\n", nameValueList->nameValue[k].value);
                            UDA_LOG(UDA_LOG_DEBUG, "residual %s\n", &pp[lstr]);
                            UDA_LOG(UDA_LOG_DEBUG, "Modified Original %s\n", replace.c_str());

                            free(original);
                            free(nameValueList->nameValue[i].value);
                            nameValueList->nameValue[i].value = strdup(replace.c_str());

                            break;
                        }
                        free(original);
                    }
                }
            }
        }
    }
    free_name_value_list(&newNameValueList);
}
