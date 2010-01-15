require 'efl'
e = ecore_evas.new();
r = e:rectangle();
r:hide();
r:show();
print(r);
r:delete();
print(r);
