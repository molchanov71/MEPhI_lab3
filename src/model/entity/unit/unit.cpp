#include "model/entity/unit.hpp"

void AbstractShared::grantAccess(std::string path)
{
    owning_programs.insert(std::move(path));
}

void AbstractShared::revokeAccess(const std::string &path)
{
    owning_programs.erase(path);
}
