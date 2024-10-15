#pragma once

#include <memory>
#include <iostream>

using std::shared_ptr;
using std::iostream;
using std::stringstream;

shared_ptr<iostream> DecompressWad(shared_ptr<iostream> stream);