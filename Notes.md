##
* we gonna need a direct path.
./my_shell

* quit or exit                         ->(check)

* shell has to be able to use cd & pwd ->(check)

* Implement the setting, deleting (unsetting)

  * this means that you should scan the command for a $<something> and replace it if found

* Read the PATH environment variable (from when the shell was started, not when/if it was set in xsh), and look for any command typed that is not directly implemented. Execute it when found; complain if not found.
* We need to be able to execute any type of command

* Implement "|" to separate commands and pipe output from one to the input of another.

* Implement "<" to pipe the contents of a file to the input of the command.

* Implement ">" to pipe the command output to a file.

* Implement "&" to run the command in the background. Otherwise, wait for the output.

##