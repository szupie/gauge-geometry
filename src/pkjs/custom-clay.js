module.exports = function(minified) {
  var clayConfig = this;
  var _ = minified._;
  var $ = minified.$;
  var HTML = minified.HTML;


  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
    clayConfig.getItemById('save-button').$element.set({
      '$position': 'sticky',
      '$bottom': '0',
      '$backgroundColor': '#333333'
    });
    
  });
  
};
