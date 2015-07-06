/*!
 * Ghostery for Chrome
 * http://www.ghostery.com/
 *
 * Copyright 2014 Ghostery, Inc. All rights reserved.
 * See https://www.ghostery.com/eula for license.
 */

define([
	'lib/bugdb',
	'lib/utils',
	'lib/conf'
], function (bugDb, utils, conf) {

	function matchesHostPath(roots, src_path) {
		var root,
			paths,
			i, j;

		for (i = 0; i < roots.length; i++) {
			root = roots[i];
			if (!root.hasOwnProperty('$')) { continue; }

			paths = root.$;
			for (j = 0; j < paths.length; j++) {
				if (src_path.indexOf(paths[j].path) === 0) {
					return paths[j].id;
				}
			}
		}

		return false;
	}

	function matchesHost(root, src_host, src_path) {
		var host_rev_arr = src_host.split('.').reverse(),
			host_part,
			node = root,
			bug_id = false,
			nodes_with_paths = [],
			i;

		for (i = 0; i < host_rev_arr.length; i++) {
			host_part = host_rev_arr[i];

			// if node has domain, advance and try to update bug_id
			if (node.hasOwnProperty(host_part)) {
				// advance node
				node = node[host_part];
				bug_id = (node.hasOwnProperty('$') ? node.$ : bug_id);

				// we store all traversed nodes that contained paths in case the final
				// node does not have the matching path
				if (src_path !== undefined && node.hasOwnProperty('$')) {
					nodes_with_paths.push(node);
				}

			// else return bug_id if it was found
			} else {
				// handle path
				if (src_path !== undefined) {
					return matchesHostPath(nodes_with_paths, src_path);
				}

				return bug_id;
			}
		}

		// handle path
		if (src_path !== undefined) {
			return matchesHostPath(nodes_with_paths, src_path);
		}

		return bug_id;
	}

	// can still produce false positives (when something that
	// matches a tracker is in the path somewhere, for example)
	function matchesRegex(src) {
		var regexes = bugDb.db.patterns.regex;

		for (var bug_id in regexes) {
			if (regexes[bug_id].test(src)) {
				return +bug_id;
			}
		}

		return false;
	}

	function matchesPath(src_path) {
		var paths = bugDb.db.patterns.path;

		// NOTE: we re-add the "/" in order to match patterns that include "/" [GHOST-1144]
		src_path = '/' + src_path;

		for (var path in paths) {
			if (src_path.indexOf(path) >= 0) {
				return paths[path];
			}
		}

		return false;
	}

	// THIS HAS TO BE SUPER FAST
	function isBug(src, tab_url) {
		var db = bugDb.db,
			found = false;

		src = utils.processUrl(src);

		found =
			// pattern classification 2: check host+path hash
			matchesHost(db.patterns.host_path, src.host, src.path) ||
			// class 1: check host hash
			matchesHost(db.patterns.host, src.host) ||
			// class 3: check path hash
			matchesPath(src.path) ||
			// class 4: check regex patterns
			matchesRegex(src.host_with_path);

		// check firstPartyExceptions
		if (conf.ignore_first_party &&
			found !== false &&
			db.firstPartyExceptions[found] &&
			utils.fuzzyUrlMatcher(tab_url, db.firstPartyExceptions[found])) {
			return false;
		}

		return found;
	}

	return {
		isBug: isBug
	};

});
