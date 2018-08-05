# How to import reference projects into the workspace #


# Precondition # 
This instruction assumes that the name of your OSTC4 clone in your workspace is `ostc4`
If this is not the case follow the instructions of chapter "Adapt repository location" below 

# Import steps #

- Select `File` => `Import..` => `General` => `Existing project into workspace` => `Next`
- Select the `RefPrj` folder of your clone
- Select the projects you want to import in the projects list
- Check the box `Copy projects into workspace`
- Next
- Finish

# Adapt repository location #

Only necessary if precondition is not met
Please update this instruction if you know a better solution to add the repository path as relativ link.

- Select top level project => `Properties` => `Resources` => `Linked Resources`
- Edit `OSTC4` variable: Replace `${WORKSPACE_LOC\ostc4` with ${WORKSPACE_LOC}\<YourRepositoryPath>
- Select top level project => `Properties` => `C/C++ Build` => `Settings` => `MCU GCC Linker` => `General`
- Edit linker script location: Change `${ProjDirPath}\..\ostc4\Small_CPU\CPU2-RTE.ld` to ${ProjDirPath}\..\<YourRepositoryLocation>\Small_CPU\CPU2-RTE.ld