# My git use

1) Work on develop: millions of commits
2) When a develop commit is supposed to be mature or stable enough, then merge it on master, like so:
   git checkout master
   git merge --squash develop
3) create new tag on master / commit / push / build / deliver 🢂 See make help

See [here](https://stackoverflow.com/questions/61009321/git-merge-only-the-last-commit-of-a-development-branch) and [there](https://stackoverflow.com/questions/5308816/how-can-i-merge-multiple-commits-onto-another-branch-as-a-single-squashed-commit) for use of git merge --squash

