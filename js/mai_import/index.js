exports.register = function (server, conf, next) {
  require('./mai.routes')(server, conf);
  next();
};

exports.register.attributes = {
  pkg: require('./package.json')
};