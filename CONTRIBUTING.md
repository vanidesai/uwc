# Contributing to IEdgeInsights / UWC
## Pull Requests

Everybody can propose a Pull Request (PR) but only the
core-maintainers of the project can merge it.

### Commit and PR Hygiene

The following points will be especially looked at during the review.

1. **master** is the default branch, it is advised always to work on a new
   feature/bug fix developer branch by keeping your local **master** branch
   untouched. The below convention while naming our branches can be followed:
   * Feature branches - feature/developer_name/feature_name
   * Bugfix branch - bugfix/developer_name/jira_bug_ids

   More details on best git branching model
   [https://nvie.com/posts/a-successful-git-branching-model/](https://nvie.com/posts/a-successful-git-branching-model/)

2. Once your branch is created by following the above convention
   (`git checkout -b <branch_name>`) and it's ready with the changes,
   run the below commands:
   * `git commit -s` - commit the message just like the old fashioned
     way with one liner and the body. Commit message
     format below:

     ```sh
      <module_name>: one liner about the commit

      Additional details about the commit message

      Signed-off by: abc <abc@intel.com>
     ```

     Here, `module_name` loosely refers to the folder name where the major
     change is been introduced.

   * `git push origin -f <branch_name>` - pushes your changes to the remote
     branch name (orgin/<branch_name>)
   * Create a pull request in github
   * If one notices any conflicts after creating a pull request, just
     apply the latest master to your <branch_name> by running commands:
      * `git checkout master` - with your current branch being <branch_name>
      * `git pull` - pull latest remote master commits to your local master
        branch
      * `git checkout <branch_name>` - Get back to your branch
      * `git rebase master` - rebase your branch with latest master.
          Fix any merge conflicts during rebasing, if any.

3. After addressing code review comments, do a `git commit --amend` to amend
   the commit message and do a `git push -f origin <branch_name>` to forcely
   push your changes. This is needed as we want to maintain a single commit.
### Review Process

The reviewers may be busy. If they take especially long to react, feel free to
trigger them by additional comments in the PR thread.

It is the job of the developer that posts the PR to rebase the PR on the target
branch when the two diverge.

Below are some additional stuff that developers should adhere to:

* Please batch all your comments by adding to `review` by clicking on `Start a
  review` to start and add all further comments by clicking on `Add to review`.
  Once done adding all the review comments, click on `Finish review` and
  subsequently on `Submit review` buttons to submit all of your review comments.
  This way all the reviewers involved will get notified only once through an email
  notification.

* In a pull request (i.e., on a feature/bugfix branch) one can have as many
  commits as possible. If all the commits are related to a single feature (eg:
  one has addressed review comments or fixed something etc.,), just ensure the
  `Title` and `Description` of the pull-request is up-to-date with respect to
  what is been added/modified/removed etc., This way, the maintainer
  during merging your pull request will squash all your commits into one and
  modify the squashed commit message to have both the `Title` and `Description`
  of the pull request.

  However, multiple commits with duplicate similiar names should not be created in a PR. 
  Whereever it makes sense, separate commits can be maintained for modularity and at the
  same time the commits can be logically grouped by the developer wherever possible.

  If pull-request has a single commit, then automatically it gets picked up in
  your pull-request description.

  If one wants to just keep ammending the single commit, he/she can do so.