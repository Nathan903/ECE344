git remote set-url origin ssh://ug250.eecg.toronto.edu/srv/ece344s/os-060/ece344
git tag asst3-end
git push --tags
git remote set-url origin git@github.com:Nathan903/ECE344.git
git push --force
git push --tags --force
cd ../marker3
os161-tester -m  3

