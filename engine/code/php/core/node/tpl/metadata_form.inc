<h2>Edit Metadata</h2>
<div class="shadow-hint">Here you can edit this node's metadata. Only change those items if you <b>really</b> know what you are doing!</div>
[INPUT|title=author|disabled|id=edit_author|required|name=edit-author|value={ATTRIBUTE|name=author|nowiki}|nowiki:metadata-form]
[INPUT|title=path|disabled|id=edit_path|required|name=edit-path|value={ATTRIBUTE|name=realname|nowiki}|nowiki:metadata-form]
[INPUT|title=time|id=edit_time|required|name=edit-time|value={ATTRIBUTE|name=time|nowiki}|type=time|nowiki:metadata-form]
[INPUT|title=date|id=edit_date|required|name=edit-date|value={ATTRIBUTE|name=date|nowiki}|type=date|nowiki:metadata-form]
[INPUT|title=thumbnail|id=edit_thumbnail|type=hidden|name=edit-thumbnail|value={ATTRIBUTE|name=thumbnail|nowiki}|nowiki:metadata-form]
[INPUT|disabled|title=father|id=edit_father|required|name=edit-father|value={ATTRIBUTE|name=father}|nowiki:metadata-form]
[FORM:metadata-form]