#include "../render_utils.h"
#include "../../../../utils/helpfulUtils.h"

int main(int argc, char **argv) {
    ctemplate::TemplateDictionary dict("2");
    dict.SetValue("NAME", "John Smith"); // the var want expand
    int winnings = rand() % 100000;

    dict.SetIntValue("VALUE", winnings);
    dict.SetFormattedValue("TAXED_VALUE", "%.2f", winnings * 0.83);
    // For now, assume everyone lives in CA.
    // (Try running the program with a 0 here instead!)
    if (true) {
        dict.ShowSection("IN_CA");
    }
    const char *file_path = "../bin/render/example.html";
    int fd;
    fd = open(file_path, O_RDWR);
    const char *new_file_path = "../bin/render/out.html";

    int newfd;
    newfd = open(file_path, O_RDWR | O_CREAT);

    auto newfileSize = GetFileSize(file_path);
    auto addr = mmap(nullptr, newfileSize, PROT_READ, MAP_PRIVATE, newfd, 0);

    std::string output;
    ctemplate::Template *tpl;

    tpl = ctemplate::Template::GetTemplate(file_path, ctemplate::DO_NOT_STRIP);
    tpl->Expand(&output, &dict);

//    ctemplate::ExpandTemplate(, ctemplate::DO_NOT_STRIP, &dict, &output);
    std::cout << output;
    return 0;
}