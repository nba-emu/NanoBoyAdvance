#!/bin/sh

if [ "$1" != "" ]; then
	MESSAGE="$1"
else
	echo "Usage: ./commit.sh $message $push (y/N)"
	exit
fi

echo "Commiting with message: $MESSAGE"
git add .
git commit -m "$MESSAGE"

if [ "$2" == "y" ]; then
	echo "Pushing..."
	git push
else
	echo "Commit was NOT pushed."
fi

