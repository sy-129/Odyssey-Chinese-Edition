#pragma once
#include "RemoteMemory.hpp"
#include "CoordinateAccess.hpp"
#include <mach/mach.h>
#include <sys/types.h>
#include <string>
class MachSession final : public ts::WritableMemory {
 mach_port_t task_=MACH_PORT_NULL;
public:
 ~MachSession() override;
 MachSession()=default;MachSession(const MachSession&)=delete;MachSession& operator=(const MachSession&)=delete;
 void close();
 bool attach(pid_t pid,std::string& error);
 bool write(std::uint64_t,const void*,std::size_t) override;
 std::vector<ts::Region> regions(std::string& error) const;
 ts::ReadResult read(std::uint64_t,void*,std::size_t) const override;
 std::optional<ts::Image> findGameImage(std::string& error) const;
 static pid_t findProcess(const std::string&,std::string& error);
};
