#!/bin/bash
REPO=greatscottgadgets/hackrf-nightly
PUBLICATION_BRANCH=master
# set -x
cd $HOME
# Checkout the branch
git clone --branch=$PUBLICATION_BRANCH https://${GITHUB_TOKEN}@github.com/$REPO.git publish
cd publish
# Update pages
cp $ARTEFACT_BASE/$BUILD_NAME.tar.xz .
# Write index page
cd $TRAVIS_BUILD_DIR
COMMITS=`git log --oneline | awk '{print $1}'`
cd $HOME/publish
echo "
<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">
<html><head>
	<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">
	<title>HackRF Nightly Builds</title>
</head>
<body>
<h2>HackRF Nightly Builds</h2>
" > index.html

URL=https://greatscottgadgets.github.io/hackrf-nightly

for commit in $COMMITS; do
    FILENAME=`find . -maxdepth 1  -name "*-$commit.tar.xz"`
    if [ "$FILENAME" != "" ]; then
        FN=${FILENAME:2}
        echo "<a href=\"$URL/$FN\">$FN</a><br />" >> index.html
    fi
    
done

echo "
</body></html>
" >> index.html

# Commit and push latest version
git add $BUILD_NAME.tar.xz index.html
git config user.name  "Travis"
git config user.email "travis@travis-ci.org"
git commit -m "Build products for $SHORT_COMMIT_HASH, built on $TRAVIS_OS_NAME, log: $TRAVIS_BUILD_WEB_URL"
if [ "$?" != "0" ]; then
    echo "Looks like the commit failed"
fi
git push -fq origin $PUBLICATION_BRANCH