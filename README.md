trovemap
========

This is an entry for GovHack. TroveMap allows you to navigate a graph of word maps, which has been generated from sydney morning herald newspaper articles, using search terms from the queensland library. A connection between multiple words is made when they are found to often coexist on the same article. The edges are further filtered to a certain threshold, and per edge as well.

Processor.cpp goes through trove articles and downloads them. This took a pretty long time, but after they're downloaded, they're cached on disk, and processing them yields a graph file.

The only keywords recognized are search phrases that exist in the list provided by the Queensland library. This means that the graph ends up being a mapping of the interests of the public, mixed with the realities of the time. For example, the word "palestine" is next to "immigrants", and "jerusalem" isn't very far away in the graph as well.

The main hairball of the graph are often very common topics/words, and world-war 2 related topics. However, there are cliques of topics formed from the newspaper articles, such as the planets (mercury, jupiter) and classical musicians (beethoven, mozart etc).

There are sometimes an excessive lot of names in some of the articles, which skews the importance of certain words. For example, birth lists, death lists, and things like that. I've tried my best to classify and ignore those articles, but there may be a way to tell the trove API not to return those. Ran out of time and couldn't find out.

The history slider at the bottom will color the nodes differently and have them have different size / opacity. The color reflects the change of occurence from the previous period, while opacity / size reflects raw numbers. You can see interesting trends there, for example the sudden spike of mention of "births" during january 1946. If you tap on a node during this mode, you can see their frequency history at the bottom.

Disclaimer:

The graph results are a bit off -- there are missing words and links. Most likely an algorithm tweak is needed to fix it, but I ran out of time.

There should have been a UI for accessing the trove articles inside the app - but for now, transferring to the iOS safari app is the way to go. I ran out of time as well.
