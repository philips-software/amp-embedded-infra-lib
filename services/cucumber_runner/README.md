docker build -t amp-cucumber-runner .
docker run -it --rm -v ${LOCAL_WORKSPACE_FOLDER}/services/cucumber_runner/features:/features amp-cucumber-runner cucumber
