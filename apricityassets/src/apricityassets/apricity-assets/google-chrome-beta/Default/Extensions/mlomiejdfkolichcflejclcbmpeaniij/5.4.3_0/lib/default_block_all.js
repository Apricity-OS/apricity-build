/*!
 * Ghostery for Chrome
 * http://www.ghostery.com/
 *
 * Copyright 2014 Ghostery, Inc. All rights reserved.
 * See https://www.ghostery.com/eula for license.
 */

define([
	'jquery'
], function ($) {

	var timer_id;

	function notifyAndSet() {
		$('#block-default-message').fadeIn({
			duration: 'fast',
			complete: function () {
				timer_id = window.setTimeout(function () {
					timer_id = null;

					$('#block-default-message').fadeOut({
						duration: 'fast',
						complete: function () {
							$('#block-by-default')[0].checked = true;
						}
					});
				}, 5000);
			}
		});
	}

	function init(saved_block_by_default, saved_all_selected) {
		// TODO repeated clicks create multiple timers
		$('#select-all').click(function () {
			if (!saved_block_by_default && !saved_all_selected && !$('#block-by-default')[0].checked) {
				notifyAndSet();
			}
		});

		// TODO anything that rerenders the DOM with the checkbox (checking
		// checkboxes, expanding the row, ...), loses the click listener
		//
		// TODO what about clicks on individual trackers?
		$('.cat-checkbox').click(function () {
			if (!saved_block_by_default && !saved_all_selected && !$('#block-by-default')[0].checked) {
				// TODO we should be examining the category model (?) instead,
				// since the browser might be filtered and category checkboxes
				// reflect combined state of visible trackers, not all trackers
				var cats = $(".cat-checkbox");
				if (cats.filter(':checked').length == cats.length) {
					notifyAndSet();
				}
			}
		});

		// TODO review
		$('#block-default-disable').click(function (e) {
			if (timer_id) {
				window.clearTimeout(timer_id);
				timer_id = null;
			}
			$('#block-default-message').fadeOut({
				duration: 'fast',
				complete: function () {
					// TODO
					//$(this).remove();
					$('#block-by-default')[0].checked = false;
				}
			});
			e.preventDefault();
		});

		// TODO review
		$('#block-default-msg-close').click(function () {
			if (timer_id) {
				window.clearTimeout(timer_id);
				timer_id = null;
			}
			$('#block-default-message').fadeOut({
				duration: 'fast',
				complete: function () {
					// TODO
					//$(this).remove();
					$('#block-by-default')[0].checked = true;
				}
			});
		});
	}

	return {
		init: init
	};

});
