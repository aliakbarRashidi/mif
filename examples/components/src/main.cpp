//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     04.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// MIF
#include <mif/application/application.h>

class Applicatin
    : public Mif::Application::Application
{
public:
    Applicatin(int argc, char const **argv)
        : Mif::Application::Application{argc, argv}
    {
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Applicatin>(argc, argv);
}
