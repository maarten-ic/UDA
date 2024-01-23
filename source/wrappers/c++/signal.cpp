#include "signal.hpp"

#include <boost/format.hpp>
#include <clientserver/stringUtils.h>
#include <uda.h>

void uda::Signal::put() const
{
    // Open file
    const char* fmt = "putdata::open(filename='%1%.nc', conventions='FUSION', class='%2%', shot='%3%', pass='%4%', "
                      "comment='%5%', /create)";
    std::string query = (boost::format(fmt) % alias_ % signal_class_ % shot_ % pass_ % comment_).str();
    udaPutAPI(query.c_str(), NULL);

    udaPutAPI("", NULL);

    udaPutAPI("", NULL);

    udaPutAPI("", NULL);

    udaPutAPI("", NULL);

    udaPutAPI("", NULL);

    udaPutAPI("", NULL);
}
