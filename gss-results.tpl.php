<?php

/**
 * @file search-results.tpl.php
 * Default theme implementation for displaying search results.
 *
 * This template collects each invocation of theme_gss_result(). This and
 * the child template are dependant to one another sharing the markup for
 * definition lists.
 *
 * Note that modules may implement their own search type and theme function
 * completely bypassing this template.
 *
 * Available variables:
 * - $search_results: All results as it is rendered through
 *   search-result.tpl.php
 *
 * @see template_preprocess_gss_results()
 */

$params = array();
foreach( $_GET as $param => $value ) {
    $params[$param] = urlencode(trim($value));
}

function buildParamlist($params) {
    $url = '?';
    foreach($params as $k=>$v) {
        $url .= sprintf("%s=%s&", $k, $v);
    }

    echo rtrim($url, '&');
}

$rel = $params;
unset($rel['sort']);

$date = $params;
$date['sort'] = 'date';

$sort = (isset($_GET['sort']) && $_GET['sort'] == 'date') ? true : false;
?>
<div id="search-results">

    <div class="nav navbar-right search-sort">
        <div class="dropdown">
            <button class="btn btn-default dropdown-toggle" type="button" id="dropdownMenu1" data-toggle="dropdown">
                Sort by
                <span class="caret"></span>
            </button>
            <ul class="dropdown-menu" role="menu" aria-labelledby="dropdownMenu1">
                <li role="presentation"<?php echo (!$sort) ? ' class="active"' : '' ?>><a role="menuitem" tabindex="-1" href="<?php buildParamlist($rel); ?>">Relevance</a></li>
                <li role="presentation"<?php echo ($sort) ? ' class="active"' : '' ?>><a role="menuitem" tabindex="-1" href="<?php buildParamlist($date); ?>">Date</a></li>
            </ul>
        </div>
    </div>

    <?php print strip_tags($head); ?>

    <div class="row">
        <div class="col-md-9">
            <ol class="google-search-results media-list">
                <?php print $search_results; ?>
            </ol>
            <?php print $pager; ?>
        </div>
    </div>

</div>

<script type="text/javascript">
    jQuery(function($) {
        var check = function(sel) {
            if (sel.val().length > 0) sel.addClass('dirty');
            else sel.removeClass('dirty');
        }
        $('#edit-keys').on('keyup', function() {
            check($(this));
        })
        check($('#edit-keys'));
    }(jQuery));
</script>