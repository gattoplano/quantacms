<h2>Edit Metadata</h2>
[SHADOW_DESCRIPTION:Here you can edit this node's metadata. Only change those items if you <b>really</b> know what you are doing!]
[FORM|name=metadata-form:
    [INPUT|type=autocomplete|node=_users|title=author|id=edit_author|required|name=edit-author|default={USER_ATTRIBUTE|name=username}|value={ATTRIBUTE|name=author|no_qtags}:metadata-form]
    [INPUT|title=time|id=edit_time|required|name=edit-time|value={ATTRIBUTE|name=time|no_qtags}|type=time:metadata-form]
    [INPUT|title=date|id=edit_date|required|name=edit-date|value={ATTRIBUTE|name=date|no_qtags}|type=date:metadata-form]
    [INPUT|title=weight|id=edit_weight|required|name=edit-weight|value={ATTRIBUTE|name=weight|no_qtags}|type=number:metadata-form]
]