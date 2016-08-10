var Boom = require('boom');
var _= require('underscore');

module.exports = function (server, conf) {

	var handler = {};

	handler.mai = {};

	handler.mai.importQuantification = function(req, rep){
		server.methods.clusterprovider.getDocument(req.params.id)
		.then(function(job){
			var inputs = job.inputs;
			var outputs = job.outputs;

			if(outputs.length > 0 && job._attachments && job._attachments[outputs[0].name]){
				return server.methods.clusterprovider.getDocumentAttachment(job._id + "/" + outputs[0].name)
				.then(function(mai){
					var codename = undefined;
					_.each(inputs, function(input){

						if(input.remote && input.remote.serverCodename){
							codename = input.remote.serverCodename;
						}
					});

					return server.methods.clusterprovider.uploadDocuments(JSON.parse(mai), codename);
				})
			}else{
				return Boom.notFound("Attachment not found");
			}

		})
		.then(rep)
		.catch(function(e){
			rep(Boom.badImplementation(e));
		});
	}	

	return handler;
}
