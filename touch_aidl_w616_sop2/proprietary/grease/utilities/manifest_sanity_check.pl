#!/usr/bin/env perl
#
# manifest_sanity_check.pl - Sanity check Grease attributes in a manifest
#
# usage: $0 [grease-config.xml OR grease/distribution bare repo] [manifest]
#
# The exit code == 0 when the sanity check passes.
#
# examples:
#   manifest_sanity_check.pl ./grease_config.xml ./default.xml
#   manifest_sanity_check.pl ./distribution ./default.xml
#
# This script performs the following checks:
#     1. Check that the default manifest revision is the same
#     as the quic-manifest-branch entnry in grease config.
#     2. Check that the grease destination branch for every project
#     is named something like <CDR-customer>/<quic-manifest-branch>.
#     3. Check that there is a grease config entry for every
#     grease-branch-id refered to by the manifest file.
#     4. Check that there are attributes x-ship, x-quic-dist
#     and x-grease-branch-id for every project in the manifest.
#     5. Check that the manifest includes the repo
#     platform/vendor/qcom-proprietary/grease/utilities.

use strict;
use warnings;

use XML::Simple;
use Data::Dumper;

my $gr;
if (-d "$ARGV[0]") {
    my $grease_config =
        `git --git-dir="$ARGV[0]" --bare show master:grease_config.xml`;
    $gr = XMLin($grease_config);
} else {
    $gr = XMLin($ARGV[0]);
}
my $mf = XMLin($ARGV[1]);

my @required_attributes = qw( x-ship x-quic-dist x-grease-branch-id );
my $mf_branch = $mf->{'default'}->{'revision'};
# Stip of trailing point revision used in RBs
# so that mf_branch is correct.
$mf_branch =~ s/(rb\d+)\.\d+$/$1/;
my $gr_branches_of; # project -> grease-branch -> 1
my $quic_mf_branches; # branch -> 1
my $exit_code = 0;
my $no_grease_config_branch_ids; # branch-id -> 1
my $quic_manifest_mismatch; # branch-id -> configured-quic-mf -> 1
my $missing_metadata; # project -> missing-attribute -> 1


foreach my $project (keys %{$mf->{'project'}}) {
    foreach my $attribute (@required_attributes) {
        if (not (exists $mf->{'project'}->{$project}->{$attribute})) {
            $missing_metadata->{$project}->{$attribute} = 1;
        }
    }
    next unless exists $mf->{'project'}->{$project}->{'x-grease-branch-id'};
    my @branch_ids = split(/,/,
        $mf->{'project'}->{$project}->{'x-grease-branch-id'});
    foreach my $branch_id (@branch_ids) {
        next if ($branch_id eq "none");
        if (not exists $gr->{'branch'}->{$branch_id}) {
            $no_grease_config_branch_ids->{$branch_id} = 1;
            next;
        }
        my $quic_mf_branch =
            $gr->{'branch'}->{$branch_id}->{'quic-manifest-branch'};
        $quic_mf_branches->{$quic_mf_branch} = 1;
        my $branch =
            $gr->{'branch'}->{$branch_id}->{'grease-manifest-branch'};
        $gr_branches_of->{$project}->{$branch} = 1;
        if ($quic_mf_branch ne $mf_branch) {
            $quic_manifest_mismatch->{$branch_id}->{$quic_mf_branch} = 1;
        }
    }
}

# Check 2.
foreach my $project (keys %{$gr_branches_of}) {
    foreach my $branch (keys %{$gr_branches_of->{$project}}) {
        next unless ($branch !~ /$mf_branch$/);
        print "warning: $project will be pushed to $branch " .
              "but it should probably be pushed to <CDR-customer>/$mf_branch\n";
        $exit_code = 1;
    }
}

# Check 3.
foreach my $branch_id (keys %{$no_grease_config_branch_ids}) {
    print "warning: no Grease config entry for $branch_id.\n";
    $exit_code = 1;
}

# Check 1.
foreach my $branch_id (keys %{$quic_manifest_mismatch}) {
    foreach my $quic_mf_branch (keys %{$quic_manifest_mismatch->{$branch_id}}) {
        print "error: Branch ID `$branch_id' in QuIC manifest branch `$mf_branch'" .
              "is configured with quic-manifest-branch `$quic_mf_branch'.\n";
        $exit_code = 1;
    }
}

# Check 4.
foreach my $project (keys %{$missing_metadata}) {
    foreach my $attribute (keys %{$missing_metadata->{$project}}) {
        print "error: `$project' is missing $attribute\n";
        $exit_code = 1;
    }
}

# Check 5.
my $utilities_repo = 'platform/vendor/qcom-proprietary/grease/utilities';
(print "error: grease utilities repo is missing\n" and
 $exit_code = 1)
    unless (exists $mf->{'project'}->{$utilities_repo});

exit $exit_code;
