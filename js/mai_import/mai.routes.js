module.exports = function (server, conf) {

  var Joi = require('joi');

  var handlers = require('./mai.handlers')(server, conf);

  server.route({
    path: '/mai/quantification/{id}', 
    method: 'PUT',
    config: {
      auth: {
               strategy: 'token',
               scope: ['clusterpost']
           },
      handler: handlers.mai.importQuantification, 
      validate: {
        params: {
          id: Joi.string().alphanum().required(),

        }, 
        query: false, 
        payload: false
      },
      description: 'Proxy to the mai application'
    }
  });

}
