//
// Copyright © Mason McParlane
//
// Helper functions for fs.c.
//

mergeInto(LibraryManager.library, {
    wch_fs_mount: function(real, virt) {
	var r = Pointer_stringify(real);
	var v = Pointer_stringify(virt);
	
	FS.mkdir(v);
	FS.mount(NODEFS, {root: r}, v);
	
	return virt;
    },
    wch_fs_unmount: function(path) {
	path = Pointer_stringify(path);
	FS.unmount(path);
    },
    wch_fs_mkdir: function(path) {
	path = Pointer_stringify(path);
	FS.mkdir(path);
    },
    wch_fs_rmdir: function(path) {
	path = Pointer_stringify(path);
	FS.rmdir(path);
    },
    wch_fs_ls: function(path, type) {
	path = Pointer_stringify(path);
	type = Pointer_stringify(type);
	
	var r = [];
	FS.readdir(path).forEach(
	    function(file) {
		if (file === '.' || file === '..') return;
		file = (path['endsWith']('/') ? path : path + '/') + file;
		var s = FS.stat(file);
		if (s) {
		    switch(type) {
		    case 'dir':
		        if (FS.isDir(s.mode)) r.push(file);
		        break;
		    case 'file':
		        if (FS.isFile(s.mode)) r.push(file);
		        break;
		    case 'all':
		        r.push(file);
		        break;
		    default:
		        throw new Error('ls: unsupported file type "' + type + '"specified');
		        break;
		    }
		}
	    });
	return allocate(intArrayFromString(r.join('\n').concat('\n')),
			'i8',
			ALLOC_NORMAL);
    },
    wch_fs_pwd: function() {
	return allocate(intArrayFromString(FS.cwd()),
			'i8',
			ALLOC_NORMAL);
    },
});
