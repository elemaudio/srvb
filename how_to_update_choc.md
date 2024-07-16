Important to update CHOC that is used in the native/choc submodule and also probably in Elementary submodule

here is copilot guide.


cristianvogel: npm command to install everything including git submodules

GitHub Copilot: ```bash
npm install && git submodule update --init --recursive
```

cristianvogel: remove all cache of git submodules

GitHub Copilot: ```bash
git submodule deinit -f --all && git rm -rf .git/modules
```

cristianvogel: fatal: No url found for submodule path 'native/choc' in .gitmodules


There is no url registred in .gitmodules file

GitHub Copilot: To resolve the issue of a missing URL for the submodule path 'native/choc' in the [`.gitmodules`](command:_github.copilot.openRelativePath?%5B%7B%22scheme%22%3A%22file%22%2C%22authority%22%3A%22%22%2C%22path%22%3A%22%2FUsers%2Fcristianvogel%2FDesktop%2FProgramming%2FSRVB-withCablesUI%2Fsrvb-fork%2F.gitmodules%22%2C%22query%22%3A%22%22%2C%22fragment%22%3A%22%22%7D%5D "/Users/cristianvogel/Desktop/Programming/SRVB-withCablesUI/srvb-fork/.gitmodules") file, you need to add the submodule information for 'native/choc' to the [`.gitmodules`](command:_github.copilot.openRelativePath?%5B%7B%22scheme%22%3A%22file%22%2C%22authority%22%3A%22%22%2C%22path%22%3A%22%2FUsers%2Fcristianvogel%2FDesktop%2FProgramming%2FSRVB-withCablesUI%2Fsrvb-fork%2F.gitmodules%22%2C%22query%22%3A%22%22%2C%22fragment%22%3A%22%22%7D%5D "/Users/cristianvogel/Desktop/Programming/SRVB-withCablesUI/srvb-fork/.gitmodules") file. This involves specifying the path and URL of the submodule. Here's how you can do it:

1. Open the [`.gitmodules`](command:_github.copilot.openRelativePath?%5B%7B%22scheme%22%3A%22file%22%2C%22authority%22%3A%22%22%2C%22path%22%3A%22%2FUsers%2Fcristianvogel%2FDesktop%2FProgramming%2FSRVB-withCablesUI%2Fsrvb-fork%2F.gitmodules%22%2C%22query%22%3A%22%22%2C%22fragment%22%3A%22%22%7D%5D "/Users/cristianvogel/Desktop/Programming/SRVB-withCablesUI/srvb-fork/.gitmodules") file in your editor.
2. Add the following lines to register the 'native/choc' submodule:

```properties
[submodule "choc"]
	path = native/choc
	url = <URL_TO_CHOC_GIT_REPOSITORY>
```

Replace `<URL_TO_CHOC_GIT_REPOSITORY>` with the actual URL of the 'choc' Git repository.

3. Save the `.gitmodules` file.
4. Run the following commands in your terminal to synchronize your local configuration:

```bash
git submodule init
git submodule update
```

This will initialize the new submodule and clone it into the specified path.

cristianvogel: i am trying to remove this submodule referenence but it dosn't seem to de-install

GitHub Copilot: To remove a submodule reference, such as `elementary` or `juce`, from your Git repository, you need to follow these steps. Let's use `elementary` as an example:

1. **Delete the submodule entry from [`.gitmodules`](command:_github.copilot.openRelativePath?%5B%7B%22scheme%22%3A%22file%22%2C%22authority%22%3A%22%22%2C%22path%22%3A%22%2FUsers%2Fcristianvogel%2FDesktop%2FProgramming%2FSRVB-withCablesUI%2Fsrvb-fork%2F.gitmodules%22%2C%22query%22%3A%22%22%2C%22fragment%22%3A%22%22%7D%5D "/Users/cristianvogel/Desktop/Programming/SRVB-withCablesUI/srvb-fork/.gitmodules") file**: Open the [`.gitmodules`](command:_github.copilot.openRelativePath?%5B%7B%22scheme%22%3A%22file%22%2C%22authority%22%3A%22%22%2C%22path%22%3A%22%2FUsers%2Fcristianvogel%2FDesktop%2FProgramming%2FSRVB-withCablesUI%2Fsrvb-fork%2F.gitmodules%22%2C%22query%22%3A%22%22%2C%22fragment%22%3A%22%22%7D%5D "/Users/cristianvogel/Desktop/Programming/SRVB-withCablesUI/srvb-fork/.gitmodules") file and remove the section related to `elementary`.

    ```properties
    [submodule "elementary"]
    path = native/elementary
    url = https://github.com/elemaudio/elementary.git
    ```

2. **Stage the `.gitmodules` changes**: After editing the `.gitmodules` file, stage the changes for commit.

    ```bash
    git add .gitmodules
    ```

3. **Delete the submodule section from [`.git/config`](command:_github.copilot.openRelativePath?%5B%7B%22scheme%22%3A%22file%22%2C%22authority%22%3A%22%22%2C%22path%22%3A%22%2FUsers%2Fcristianvogel%2FDesktop%2FProgramming%2FSRVB-withCablesUI%2Fsrvb-fork%2F.git%2Fconfig%22%2C%22query%22%3A%22%22%2C%22fragment%22%3A%22%22%7D%5D "/Users/cristianvogel/Desktop/Programming/SRVB-withCablesUI/srvb-fork/.git/config")**: Open your [`.git/config`](command:_github.copilot.openRelativePath?%5B%7B%22scheme%22%3A%22file%22%2C%22authority%22%3A%22%22%2C%22path%22%3A%22%2FUsers%2Fcristianvogel%2FDesktop%2FProgramming%2FSRVB-withCablesUI%2Fsrvb-fork%2F.git%2Fconfig%22%2C%22query%22%3A%22%22%2C%22fragment%22%3A%22%22%7D%5D "/Users/cristianvogel/Desktop/Programming/SRVB-withCablesUI/srvb-fork/.git/config") file in a text editor and remove the submodule section related to `elementary`.

    ```ini
    [submodule "elementary"]
        url = https://github.com/elemaudio/elementary.git
    ```

4. **Unstage and remove the submodule files**: Run the following commands to unstage the submodule path and then remove the submodule directory from your working directory.

    ```bash
    git rm --cached native/elementary
    rm -rf native/elementary
    ```

5. **Remove the submodule's [`.git`](command:_github.copilot.openRelativePath?%5B%7B%22scheme%22%3A%22file%22%2C%22authority%22%3A%22%22%2C%22path%22%3A%22%2FUsers%2Fcristianvogel%2FDesktop%2FProgramming%2FSRVB-withCablesUI%2Fsrvb-fork%2F.git%22%2C%22query%22%3A%22%22%2C%22fragment%22%3A%22%22%7D%5D "/Users/cristianvogel/Desktop/Programming/SRVB-withCablesUI/srvb-fork/.git") directory**: Delete the [`.git`](command:_github.copilot.openRelativePath?%5B%7B%22scheme%22%3A%22file%22%2C%22authority%22%3A%22%22%2C%22path%22%3A%22%2FUsers%2Fcristianvogel%2FDesktop%2FProgramming%2FSRVB-withCablesUI%2Fsrvb-fork%2F.git%22%2C%22query%22%3A%22%22%2C%22fragment%22%3A%22%22%7D%5D "/Users/cristianvogel/Desktop/Programming/SRVB-withCablesUI/srvb-fork/.git") directory located in the submodule's directory. Since you've already removed the directory in the previous step, this is more of a general step if you need to manually remove any leftover [`.git`](command:_github.copilot.openRelativePath?%5B%7B%22scheme%22%3A%22file%22%2C%22authority%22%3A%22%22%2C%22path%22%3A%22%2FUsers%2Fcristianvogel%2FDesktop%2FProgramming%2FSRVB-withCablesUI%2Fsrvb-fork%2F.git%22%2C%22query%22%3A%22%22%2C%22fragment%22%3A%22%22%7D%5D "/Users/cristianvogel/Desktop/Programming/SRVB-withCablesUI/srvb-fork/.git") directories.

6. **Commit your changes**: Commit the removal of the submodule.

    ```bash
    git commit -m "Removed elementary submodule"
    ```

7. **Delete the submodule's entry from the `.git/modules` directory**: Finally, remove the submodule's data from the `.git/modules` directory.

    ```bash
    rm -rf .git/modules/native/elementary
    ```

Repeat these steps for any other submodule you wish to remove, such as `juce`. Make sure to adjust the paths and names accordingly.

cristianvogel: how do i specifiy a specific branch and commit for a submodule

GitHub Copilot: To specify a specific branch and commit for a submodule in Git, follow these steps:

1. **Navigate to the submodule directory**: First, change your current working directory to the submodule's directory.

    ```bash
    cd path/to/submodule
    ```

    Replace `path/to/submodule` with the actual path to your submodule, such as `native/elementary`.

2. **Checkout the specific branch**: Checkout the branch you want to track in the submodule.

    ```bash
    git checkout branch-name
    ```

    Replace `branch-name` with the name of the branch you want to use.

3. **Pull or reset to the specific commit**: If you want the latest commit from that branch, you can simply pull. However, if you need a specific commit, use `git reset` or `git checkout` to move to that commit.

    - To pull the latest changes (if you're tracking a branch):

        ```bash
        git pull
        ```

    - To move to a specific commit:

        ```bash
        git reset --hard commit-hash
        ```

        or

        ```bash
        git checkout commit-hash
        ```

    Replace `commit-hash` with the hash of the commit you want to use.

4. **Update the submodule reference in the main project**: After you have checked out the specific branch and commit in the submodule, navigate back to the root of your main project and stage the submodule changes.

    ```bash
    cd ../..
    git add path/to/submodule
    ```

    This adds the new state of the submodule to your main project.

5. **Commit the change**: Commit the update to the submodule reference in your main project.

    ```bash
    git commit -m "Update submodule to specific branch and commit"
    ```

6. **(Optional) Push the changes**: If you want these changes to be reflected in the remote repository, push them.

    ```bash
    git push
    ```

**Note**: If you want to ensure that the submodule is always checked out to a specific branch when others clone or pull from the main repository, you can add the `branch` option to the [`.gitmodules`](command:_github.copilot.openRelativePath?%5B%7B%22scheme%22%3A%22file%22%2C%22authority%22%3A%22%22%2C%22path%22%3A%22%2FUsers%2Fcristianvogel%2FDesktop%2FProgramming%2FSRVB-withCablesUI%2Fsrvb-fork%2F.gitmodules%22%2C%22query%22%3A%22%22%2C%22fragment%22%3A%22%22%7D%5D "/Users/cristianvogel/Desktop/Programming/SRVB-withCablesUI/srvb-fork/.gitmodules") file. This doesn't lock the submodule to a specific commit but ensures the specified branch is checked out.

```properties
[submodule "submodule-name"]
	path = path/to/submodule
	url = https://example.com/submodule.git
	branch = branch-name
```

Replace `submodule-name`, `path/to/submodule`, `https://example.com/submodule.git`, and `branch-name` with your submodule's name, path, URL, and branch name, respectively. After adding the branch to `.gitmodules`, remember to commit and push the `.gitmodules` file as well.