# Adding a new PoseidonOS (POS) CLI command

## 1. Define the syntax for your command

Define the syntax for your command in $YOUR_POS_PATH/tool/cli/cmd/pos_cli_syntax.ebnf.

 Note: command syntax must be written in Extended Backus-Naur Form (EBNF). You can find EBNF grammar here: [EBNF Wikipedia](https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form)


## 2. Add your command to POS CLI client

If you add a set of commands, add your command set as a Go package. If you add a command in the existing category, you can skip this step.

**Note:** In Go lang, a package is a set of files that share common variables and methods.

1. Create a new copy of an existing package (e.g., arraycmds or devicecmds).

2. In a package, the general.go file describes the overall structure and the help message of the command set (e.g., array command, device command, ...)

3. Add your sub-command in the init() function in the general.go file.
    ```Go
    func init() {
        // Add subcommands to array command.
        // If you create a new subcommand, add it here.
        ArrayCmd.AddCommand(ListArrayCmd)
        ArrayCmd.AddCommand(MountArrayCmd)
        ArrayCmd.AddCommand(UnmountArrayCmd)
        ArrayCmd.AddCommand(DeleteArrayCmd)
        ArrayCmd.AddCommand(CreateArrayCmd)
        ArrayCmd.AddCommand(AddSpareCmd)
        ArrayCmd.AddCommand(RemoveSpareCmd)
        ArrayCmd.AddCommand(AutocreateArrayCmd)
    }
    ```

4. Add your sub-command by adding a file (e.g., arraycmds/create_array.go). We recommend to copy an existing file.

5. Define your request and response messages

6. Add your message formats to $YOUR_POS_PATH/tool/cli/cmd/messages/requests.go and responses.go.

7. Define the output format for your command.

    If your command displays some data and requires a specific print format for the output (e.g, array list), you can define the print format for your command in the $YOUR_POS_PATH/tool/cli/displaymgr/display_response.go

8. Write a unit test for your command

    We recommend to write a unit test for your command; it will ensure that your new command is processed well. 

    **Note**: refer to the other *_test.go files.

    Type "go test -v" in the directory to test your command. Go test will display the result on the terminal.

## 3. Add your command to the CLI server

1. Add your command to the $YOUR_POS_PATH/src/cli directory. Copy an existing command file (e.g., create_array_command.h and create_array_command.cpp)

2. Add your command to $YOUR_POS_PATH/src/cli/request_handler.cpp. Add the header file created in the previous step and add your command to the command dictionary.

## 4. Build

1. Build POS using $YOUR_POS_PATH/script/build_ibofos.sh or make in the ibofos directory. The makefile will create $YOUR_POS_PATH/bin/poseidonos-cli.

    Note: if you want to build the CLI client only, execute $YOUR_POS_PATH/tool/cli/scripts/build_cli.sh. It will create the binary of the CLI client in the $YOUR_POS_PATH/tool/cli/bin directory.

## 5. Update Documents

1. When you build POS, the documents for your CLI will automatically be created/updated in $YOUR_POS_PATH/doc/guides/cli/your_command.md. The documents are generated from the help messages and the flags of your command.

2. Commit the documents to the POS repository with your CLI source code.

## 6. Generate a railroad diagram for the updated CLI

Update the railroad diagram for POS CLI ($YOUR_POS_PATH/doc/cli/pos-railroad.html) [Syntax Diagram Wikipedia](https://en.wikipedia.org/wiki/Syntax_diagram). 

Note: Currently, we generate the railroad diagram using [EBNF2RAILROAD](https://github.com/matthijsgroen/ebnf2railroad). The input is an EBNF (.ebnf) file, and the output is an HTML file.