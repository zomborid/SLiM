#!/bin/bash

echo "SLIM_HOME was \"$SLIM_HOME\""
export SLIM_HOME=$(cd `dirname $0` && pwd)
echo "SLIM_HOME is now set to \"$SLIM_HOME\""
echo Open new command window to see the changes.