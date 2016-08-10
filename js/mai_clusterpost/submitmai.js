var clusterpost = require("clusterpost-lib");
var argv = require('minimist')(process.argv.slice(2));
var inputfiles = [];
var path = require('path');

const getConfigFile = function () {
 try {
   // Try to load the user's personal configuration file
   return require(process.cwd() + '/conf.my.json');
 } catch (e) {
   // Else, read the default configuration file
   return require(process.cwd() + '/conf.json');
 }
}

var imageName = argv['f'];
var threshold = argv['t'];
threshold = "" + threshold;

if (imageName === undefined || threshold === undefined)
{
    console.error("usage: mai -f -t");
    console.error("node submitmai.js -f image file name -t threshold");
    process.exit(1);
}

inputfiles.push(imageName);
imageName = path.basename(imageName);

var conf = getConfigFile();

var job = {
        "executable": "mai",
        "parameters": [
            {
                "flag": "",
                "name": threshold //To modify
            },
            {
                "flag": "",
                "name": imageName
            }
        ],
        "inputs": [
            {
                "name": imageName
            }
        ],
        "outputs": [           
            {
                "type": "file",
                "name": "mai.json"
            },
            {
                "type": "file",
                "name": "stdout.out"
            },
            {
                "type": "file",
                "name": "stderr.err"
            }
        ],
        "type": "job",
        "userEmail": conf.user.email
    };

clusterpost.setClusterPostServer(conf.url);

clusterpost.userLogin(conf.user)
.then(function(res){
    console.log(res);
    return clusterpost.getExecutionServers();
})
.then(function(res){
    job.executionserver = res[0].name; //Or select the computing grid where you want to submit your job.
    console.log(job);
    return clusterpost.createAndSubmitJob(job, inputfiles)
})
.then(function(jobid){ 
    console.log(jobid);//Job id of the task submitted
})
.catch(console.error)
