
const Joi = require('joi');
const Lab = require('lab');
const lab = exports.lab = Lab.script();

var clusterpost = require("clusterpost-lib");
var path = require('path');
var _= require('underscore');
const request= require('request');
const Promise = require('bluebird');

const getConfigFile = function () {
 try {
   // Try to load the user's personal configuration file
   return require(process.cwd() + '/conf.my.json');
 } catch (e) {
   // Else, read the default configuration file
   return require(process.cwd() + '/conf.json');
 }
}

var conf = getConfigFile();

clusterpost.setClusterPostServer(conf.url);

lab.experiment("Test mai", function(){
    lab.test('login', function(){
        return clusterpost.userLogin(conf.user)
        .then(function(res){
            console.log(res);
            return true;
        })
    });
    lab.test('returns true when job is attached to DB', function(jobid){
        return clusterpost.getJobs("mai", 'DONE') 
        .then(function(jobs){
            var jobsid = _.pluck(jobs,'_id');
            return Promise.map(jobsid, function(jobid){
                return new Promise(function(resolve, reject){
                    var options = {
                        "url": conf.url + "/mai/quantification/" + jobid,
                        "method": "PUT",
                        "headers": {
                            authorization: "Bearer " + clusterpost.getUserToken()
                        }
                    }
                    request(options, function(err, resp, body){
                        if (err){
                            reject(err);
                        }
                        else {
                            resolve(body);
                        }
                    })
                })
            }).then(function(status){
                console.log(status);
            })
        });
    });


});
